#include "mainwindow.h"
#include <QApplication>

// Punto de entrada del programa
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);  // Inicializa la aplicación Qt

    MainWindow w;                // Crea la ventana principal
    w.show();                    // Muestra la ventana principal

    return a.exec();             // Ejecuta el bucle principal de eventos
}
