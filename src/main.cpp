#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Kernel.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace
{
constexpr UINT WM_APP_APPEND_TEXT = WM_APP + 1;
constexpr UINT WM_APP_SET_STATUS = WM_APP + 2;
constexpr UINT WM_APP_SCENARIO_DONE = WM_APP + 3;

constexpr int kBtnCpuMemory = 101;
constexpr int kBtnIo = 102;
constexpr int kQuantumInputId = 201;
constexpr int kCpuInputId = 202;
constexpr int kIoInputId = 203;
constexpr int kOutputId = 204;
constexpr int kStatusId = 205;
constexpr int kMargin = 12;
constexpr int kGap = 8;
constexpr int kButtonWidth = 160;
constexpr int kButtonHeight = 28;
constexpr int kInputHeight = 110;
constexpr int kOutputMinHeight = 220;

enum class ScenarioKind
{
    CpuMemory,
    Io
};

struct AppState
{
    HWND btnCpu = nullptr;
    HWND btnIo = nullptr;
    HWND quantumLabel = nullptr;
    HWND quantumInput = nullptr;
    HWND cpuInput = nullptr;
    HWND ioInput = nullptr;
    HWND output = nullptr;
    HWND status = nullptr;
    bool busy = false;
};

class WindowStdoutBuf final : public std::streambuf
{
public:
    explicit WindowStdoutBuf(HWND hwnd) : hwnd_(hwnd) {}

protected:
    int overflow(int ch) override
    {
        if (ch == traits_type::eof())
            return sync() == 0 ? 0 : traits_type::eof();

        buffer_.push_back(static_cast<char>(ch));
        if (ch == '\n')
            flushBuffer();
        return ch;
    }

    int sync() override
    {
        flushBuffer();
        return 0;
    }

private:
    void flushBuffer()
    {
        if (buffer_.empty())
            return;

        auto *payload = new std::string(buffer_);
        PostMessageA(hwnd_, WM_APP_APPEND_TEXT, 0, reinterpret_cast<LPARAM>(payload));
        buffer_.clear();
    }

    HWND hwnd_;
    std::string buffer_;
};

class ScopedCoutRedirect
{
public:
    explicit ScopedCoutRedirect(std::streambuf *replacement)
        : oldBuf_(std::cout.rdbuf(replacement))
    {
    }

    ScopedCoutRedirect(const ScopedCoutRedirect &) = delete;
    ScopedCoutRedirect &operator=(const ScopedCoutRedirect &) = delete;

    ~ScopedCoutRedirect()
    {
        std::cout.rdbuf(oldBuf_);
    }

private:
    std::streambuf *oldBuf_;
};

static std::string trim(const std::string &s)
{
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
        ++start;

    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;

    return s.substr(start, end - start);
}

static std::string readControlText(HWND hwnd)
{
    int len = GetWindowTextLengthA(hwnd);
    if (len <= 0) return "";
    
    std::string text(len, '\0');
    GetWindowTextA(hwnd, text.data(), len + 1);
    return text;
}

static int readPositiveInt(HWND hwnd, int fallback, std::string &error)
{
    std::string text = trim(readControlText(hwnd));
    if (text.empty())
        return fallback;

    try
    {
        size_t idx = 0;
        int value = std::stoi(text, &idx);
        if (idx != text.size() || value <= 0)
        {
            error = "El quantum debe ser un entero positivo.";
            return fallback;
        }
        return value;
    }
    catch (const std::exception&)
    {
        error = "El quantum debe ser un entero positivo.";
        return fallback;
    }
}

static void setWindowTextFromString(HWND hwnd, const std::string &text)
{
    SetWindowTextA(hwnd, text.c_str());
}

static void appendToOutput(HWND output, const std::string &text)
{
    int len = GetWindowTextLengthA(output);
    SendMessageA(output, EM_SETSEL, len, len);
    SendMessageA(output, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));
    SendMessageA(output, EM_SCROLLCARET, 0, 0);
}

static void clearOutput(HWND output)
{
    SetWindowTextA(output, "");
}

static std::vector<Kernel::CpuProcessSpec> parseCpuSpecs(const std::string &text, std::string &error)
{
    std::vector<Kernel::CpuProcessSpec> specs;
    std::istringstream input(text);
    std::string line;

    while (std::getline(input, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        std::istringstream row(line);
        Kernel::CpuProcessSpec spec;
        if (!(row >> spec.name >> spec.burstTime >> spec.memSize))
        {
            error = "Formato invalido en CPU+memoria. Usa: Nombre, Tiempo total de CPU, Memoria necesaria. Una linea por proceso.";
            return {};
        }

        specs.push_back(spec);
    }

    if (specs.empty())
        error = "Ingresa al menos un proceso para CPU+memoria.";

    return specs;
}

static std::vector<Kernel::IoProcessSpec> parseIoSpecs(const std::string &text, std::string &error)
{
    std::vector<Kernel::IoProcessSpec> specs;
    std::istringstream input(text);
    std::string line;

    while (std::getline(input, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        std::istringstream row(line);
        Kernel::IoProcessSpec spec;
        if (!(row >> spec.name >> spec.burstTime >> spec.memSize >> spec.ioAtTick))
        {
            error = "Formato invalido en E/S. Usa: Nombre, Tiempo de CPU, Memoria necesaria, Tick antes de cambio. Una linea por proceso.";
            return {};
        }

        specs.push_back(spec);
    }

    if (specs.empty())
        error = "Ingresa al menos un proceso para E/S.";

    return specs;
}

static void refreshButtons(AppState *state)
{
    const bool enabled = !state->busy;
    EnableWindow(state->btnCpu, enabled);
    EnableWindow(state->btnIo, enabled);
    EnableWindow(state->quantumInput, enabled);
    EnableWindow(state->cpuInput, enabled);
    EnableWindow(state->ioInput, enabled);
}

static void resizeLayout(HWND hwnd, AppState *state)
{
    if (!state)
        return;

    RECT rc{};
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    int topY = kMargin + 168;
    int buttonY = topY;
    int buttonTotalWidth = 2 * kButtonWidth + kGap;
    int startX = ((width - buttonTotalWidth) / 2 > kMargin) ? ((width - buttonTotalWidth) / 2) : kMargin;

    MoveWindow(state->quantumLabel, kMargin, buttonY + 5, 68, 18, TRUE);
    MoveWindow(state->quantumInput, kMargin + 86, buttonY + 2, 72, 24, TRUE);
    MoveWindow(state->btnCpu, startX, buttonY, kButtonWidth, kButtonHeight, TRUE);
    MoveWindow(state->btnIo, startX + kButtonWidth + kGap, buttonY, kButtonWidth, kButtonHeight, TRUE);

    int inputTop = buttonY + kButtonHeight + 18;
    int halfWidth = (width - 3 * kMargin) / 2;
    int inputHeight = kInputHeight;
    if (inputHeight < 110)
        inputHeight = 110;

    MoveWindow(state->cpuInput, kMargin, inputTop, halfWidth, inputHeight, TRUE);
    MoveWindow(state->ioInput, kMargin * 2 + halfWidth, inputTop, width - (kMargin * 3) - halfWidth, inputHeight, TRUE);

    int outputTop = inputTop + inputHeight + 40;
    int outputHeight = height - outputTop - kMargin;
    if (outputHeight < kOutputMinHeight)
        outputHeight = kOutputMinHeight;

    MoveWindow(state->status, kMargin, outputTop - 22, width - 2 * kMargin, 18, TRUE);
    MoveWindow(state->output, kMargin, outputTop, width - 2 * kMargin, outputHeight, TRUE);
}

static void postString(HWND hwnd, UINT message, const std::string &text)
{
    PostMessageA(hwnd, message, 0, reinterpret_cast<LPARAM>(new std::string(text)));
}

static void runScenarioAsync(HWND hwnd, ScenarioKind kind)
{
    auto *state = reinterpret_cast<AppState *>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    if (!state || state->busy)
        return;

    std::string inputText = (kind == ScenarioKind::CpuMemory) ? readControlText(state->cpuInput) : readControlText(state->ioInput);
    std::string quantumError;
    int quantum = readPositiveInt(state->quantumInput, 3, quantumError);
    if (!quantumError.empty())
    {
        setWindowTextFromString(state->status, "Estado: error de entrada");
        clearOutput(state->output);
        appendToOutput(state->output, quantumError + "\r\n");
        return;
    }

    state->busy = true;
    refreshButtons(state);
    setWindowTextFromString(state->status, "Estado: cargando la simulacion...");
    clearOutput(state->output);
    appendToOutput(state->output, "Cargando...\r\n\r\n");

    std::thread([hwnd, kind, quantum, inputText = std::move(inputText)]() mutable {
        std::string parseErrorLocal;
        if (kind == ScenarioKind::CpuMemory)
        {
            auto specs = parseCpuSpecs(inputText, parseErrorLocal);
            if (!parseErrorLocal.empty())
            {
                postString(hwnd, WM_APP_SET_STATUS, "Estado: error de entrada");
                postString(hwnd, WM_APP_APPEND_TEXT, parseErrorLocal + "\r\n");
                PostMessageA(hwnd, WM_APP_SCENARIO_DONE, 0, 0);
                return;
            }

            WindowStdoutBuf buf(hwnd);
            ScopedCoutRedirect redirect(&buf);
            Kernel kernel(quantum);
            kernel.runScenarioCPUAndMemoryContention(specs);
            std::cout.flush();
        }
        else
        {
            auto specs = parseIoSpecs(inputText, parseErrorLocal);
            if (!parseErrorLocal.empty())
            {
                postString(hwnd, WM_APP_SET_STATUS, "Estado: error de entrada");
                postString(hwnd, WM_APP_APPEND_TEXT, parseErrorLocal + "\r\n");
                PostMessageA(hwnd, WM_APP_SCENARIO_DONE, 0, 0);
                return;
            }

            WindowStdoutBuf buf(hwnd);
            ScopedCoutRedirect redirect(&buf);
            Kernel kernel(quantum);
            kernel.runScenarioSimultaneousIO(specs);
            std::cout.flush();
        }

        postString(hwnd, WM_APP_SET_STATUS, "Estado: listo");
        PostMessageA(hwnd, WM_APP_SCENARIO_DONE, 0, 0);
    }).detach();
}

static void createControls(HWND hwnd, AppState *state)
{
    HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrA(hwnd, GWLP_HINSTANCE));
    HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    HWND title = CreateWindowExA(0, "STATIC", "Mini-Kernel Demo", WS_CHILD | WS_VISIBLE, kMargin, kMargin, 420, 24, hwnd, nullptr, instance, nullptr);
    HWND subtitle = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_LEFT, kMargin, kMargin + 22, 1160, 116, hwnd, nullptr, instance, nullptr);
    state->quantumLabel = CreateWindowExA(0, "STATIC", "Quantum", WS_CHILD | WS_VISIBLE, kMargin, kMargin + 150, 68, 18, hwnd, nullptr, instance, nullptr);

    state->btnCpu = CreateWindowExA(0, "BUTTON", "CPU + memoria", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kBtnCpuMemory), instance, nullptr);
    state->btnIo = CreateWindowExA(0, "BUTTON", "E/S simultanea", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kBtnIo), instance, nullptr);
    state->quantumInput = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "3",
        WS_CHILD | WS_VISIBLE | ES_NUMBER,
        0,
        0,
        0,
        0,
        hwnd,
        reinterpret_cast<HMENU>(kQuantumInputId),
        instance,
        nullptr);

    state->cpuInput = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "P_A 10 400\r\nP_B 8 400\r\nP_C 6 300\r\nP_D 12 200\r\n",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, 0, 0,
        hwnd, reinterpret_cast<HMENU>(kCpuInputId), instance, nullptr);

    state->ioInput = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "P_1 9 100 2\r\nP_2 9 100 2\r\nP_3 6 100 3\r\n",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
        0, 0, 0, 0,
        hwnd, reinterpret_cast<HMENU>(kIoInputId), instance, nullptr);

    state->status = CreateWindowExA(0, "STATIC", "Estado: listo", WS_CHILD | WS_VISIBLE, kMargin, 0, 400, 20, hwnd, reinterpret_cast<HMENU>(kStatusId), instance, nullptr);

    state->output = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        "EDIT",
        "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | ES_LEFT,
        0, 0, 0, 0,
        hwnd, reinterpret_cast<HMENU>(kOutputId), instance, nullptr);

    const HWND controls[] = {title, subtitle, state->quantumLabel, state->btnCpu, state->btnIo, state->quantumInput, state->cpuInput, state->ioInput, state->status, state->output};
    for (HWND control : controls)
        SendMessageA(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

    SetWindowTextA(
        subtitle,
        "Proyecto por:\r\n"
        "Juan Sebastian Rodriguez Carreno - 2023102017\r\n"
        "Victor Manuel Torres Beltran - 20211020104\r\n"
        "Andres Felipe Pulido Suarez - 20211020049\r\n"
        "Janeth Oliveros Ramirez - 20182020100\r\n\r\n"
        "La simulacion funciona con dos escenarios editables.\r\n"
        "CPU + memoria: Nombre, Tiempo Total de CPU y Memoria Requerida.\r\n"
        "E/S simultanea: Nombre, Tiempo de CPU, memoria y ticks antes de pedir E/S.");

    resizeLayout(hwnd, state);
}

static void destroyState(HWND hwnd)
{
    auto *state = reinterpret_cast<AppState *>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    delete state;
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, 0);
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto *state = reinterpret_cast<AppState *>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

    switch (msg)
    {
    case WM_CREATE:
    {
        auto *created = new AppState();
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(created));
        createControls(hwnd, created);
        return 0;
    }
    case WM_SIZE:
        resizeLayout(hwnd, state);
        return 0;
    case WM_COMMAND:
        if (state && !state->busy)
        {
            switch (LOWORD(wParam))
            {
            case kBtnCpuMemory:
                runScenarioAsync(hwnd, ScenarioKind::CpuMemory);
                break;
            case kBtnIo:
                runScenarioAsync(hwnd, ScenarioKind::Io);
                break;
            }
        }
        return 0;
    case WM_APP_APPEND_TEXT:
    {
        auto *text = reinterpret_cast<std::string *>(lParam);
        if (state && state->output && text)
            appendToOutput(state->output, *text);
        delete text;
        return 0;
    }
    case WM_APP_SET_STATUS:
    {
        auto *text = reinterpret_cast<std::string *>(lParam);
        if (state && state->status && text)
            setWindowTextFromString(state->status, *text);
        delete text;
        return 0;
    }
    case WM_APP_SCENARIO_DONE:
        if (state)
        {
            state->busy = false;
            refreshButtons(state);
            setWindowTextFromString(state->status, "Estado: listo");
        }
        return 0;
    case WM_DESTROY:
        destroyState(hwnd);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}
} // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCmd)
{
    const char *className = "MiniKernelDemoWindow";

    WNDCLASSA wc{};
    wc.lpfnWndProc = wndProc;
    wc.hInstance = instance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(nullptr, "No se pudo registrar la ventana principal.", "Mini-Kernel", MB_ICONERROR | MB_OK);
        return 1;
    }

    HWND hwnd = CreateWindowExA(
        0,
        className,
        "Mini-Kernel Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1200,
        820,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!hwnd)
    {
        MessageBoxA(nullptr, "No se pudo crear la ventana principal.", "Mini-Kernel", MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(hwnd, showCmd);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageA(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return static_cast<int>(msg.wParam);
}
