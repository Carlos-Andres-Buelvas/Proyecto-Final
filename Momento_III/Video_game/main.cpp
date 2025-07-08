#include "mainwindow.h"
#include "juego2.h"
#include <QApplication>
#include <QScreen>

//#define DESARROLLO_NIVEL2  // 🔁 Comenta esta línea para volver al menú normal

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

#ifdef DESARROLLO_NIVEL2
    // 🔧 Modo desarrollo: iniciar directamente en nivel 2
    Juego2* nivel2 = new Juego2();
    nivel2->setWindowTitle("Nivel 2 - Desarrollo");
    nivel2->show();
    nivel2->setFocus();

    // Centrar ventana
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - nivel2->width()) / 2;
    int y = (screenGeometry.height() - nivel2->height()) / 2;
    nivel2->move(x, y);

#else
    // 🔁 Modo normal: iniciar en menú principal
    MainWindow w;
    w.show();
#endif

    return a.exec();
}
