#include "platform/WindowEffects.h"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QTest>

#include <windows.h>

namespace {

HWND handle(QWindow& window)
{
    return reinterpret_cast<HWND>(window.winId());
}

bool toolWindow(HWND hwnd)
{
    return (::GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) != 0;
}

bool appWindow(HWND hwnd)
{
    return (::GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_APPWINDOW) != 0;
}

bool minimizeBox(HWND hwnd)
{
    return (::GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_MINIMIZEBOX) != 0;
}

// The launcher as Main.qml's flags make it: a frameless tool window, out of
// the taskbar and Alt+Tab. These are what Windows picks those by, so the
// styles are what a test can see; the buttons themselves are the shell's.
class TaskbarWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void pinnedIsAWindowOfTheDesktop();
    void unpinnedIsAToolWindowAgain();
};

void TaskbarWindowTest::pinnedIsAWindowOfTheDesktop()
{
    QQuickWindow window;
    window.setFlags(Qt::Tool | Qt::FramelessWindowHint);
    window.setGeometry(80, 80, 320, 120);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const HWND hwnd = handle(window);
    QVERIFY(toolWindow(hwnd));
    QVERIFY(!appWindow(hwnd));
    QVERIFY(!minimizeBox(hwnd));

    ws::win::setTaskbarWindow(&window, true);
    QVERIFY(!toolWindow(hwnd));
    QVERIFY(appWindow(hwnd));
    QVERIFY(minimizeBox(hwnd));
    // Asked again on every opening: nothing changes, nothing is recreated.
    ws::win::setTaskbarWindow(&window, true);
    QCOMPARE(hwnd, handle(window));
    QVERIFY(appWindow(hwnd));
    window.close();
}

void TaskbarWindowTest::unpinnedIsAToolWindowAgain()
{
    QQuickWindow window;
    window.setFlags(Qt::Tool | Qt::FramelessWindowHint);
    window.setGeometry(80, 80, 320, 120);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const HWND hwnd = handle(window);
    ws::win::setTaskbarWindow(&window, true);
    ws::win::setTaskbarWindow(&window, false);
    QVERIFY(toolWindow(hwnd));
    QVERIFY(!appWindow(hwnd));
    QVERIFY(!minimizeBox(hwnd));
    window.close();
}

} // namespace

QTEST_MAIN(TaskbarWindowTest)
#include "tst_taskbarwindow.moc"
