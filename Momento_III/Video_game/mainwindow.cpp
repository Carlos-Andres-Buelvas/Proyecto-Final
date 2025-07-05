#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsTextItem>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>  // Incluir QMessageBox aquí también

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , juego(nullptr)
{
    ui->setupUi(this);

    // Cargar la fuente al iniciar
    dragonBallFont = loadDragonBallFont();

    // Aplicar la fuente a los botones
    QFont font(dragonBallFont, 18, QFont::Bold);
    ui->newGameButton->setFont(font);
    ui->continueButton->setFont(font);

    // Estilo adicional (opcional)
    QString buttonStyle = R"(
        QPushButton {
            background-color: #ff9900;
            color: white;
            border: 2px solid #ffcc00;
            border-radius: 15px;
            padding: 8px;
            text-shadow: 2px 2px 4px #000000;
        }
        QPushButton:hover {
            background-color: #ffaa33;
            color: black;
            border-color: #ffffff;
        }
    )";
    ui->newGameButton->setStyleSheet(buttonStyle);
    ui->continueButton->setStyleSheet(buttonStyle);
}

QString MainWindow::loadDragonBallFont() {
    QString fontPath = ":/fondos/Pictures/db_font.ttf";

    if (!QFile::exists(fontPath)) {
        qDebug() << "Error: No se encontró el archivo de fuente en" << fontPath;
        return "Arial";
    }

    int fontId = QFontDatabase::addApplicationFont(fontPath);
    if (fontId == -1) {
        qDebug() << "Error al cargar la fuente Dragon Ball";
        return "Arial";
    }

    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (fontFamilies.isEmpty()) {
        qDebug() << "La fuente no contiene familias definidas";
        return "Arial";
    }

    qDebug() << "Fuente Dragon Ball cargada correctamente:" << fontFamilies.first();
    return fontFamilies.first();
}

void MainWindow::on_newGameButton_clicked() {
    if (juego) {
        delete juego;
        juego = nullptr;
    }

    try {
        juego = new Juego();
        juego->show();

        // Conexión para manejar el regreso al menú principal
        connect(juego, &Juego::salirAlMenu, this, [this]() {
            juego->hide();
            this->show();
        });

        // Mantén tu conexión existente pero añade esto:
        connect(juego, &Juego::gameOver, this, [this]() {
            // Ocultar el juego después de mostrar el mensaje
            juego->hide();

            // Mostrar mensaje de game over con opciones
            QMessageBox msgBox;
            msgBox.setWindowTitle("Game Over");
            msgBox.setText("¡Has perdido! ¿Qué quieres hacer?");

            QPushButton *reintentarButton = msgBox.addButton("Reintentar", QMessageBox::ActionRole);
            QPushButton *menuButton = msgBox.addButton("Menú Principal", QMessageBox::ActionRole);
            msgBox.setDefaultButton(reintentarButton);

            // Estilo personalizado para el QMessageBox
            msgBox.setStyleSheet(
                "QMessageBox {"
                "   background-color: #2c3e50;"
                "   color: white;"
                "}"
                "QMessageBox QLabel {"
                "   color: white;"
                "   font: bold 16px;"
                "}"
                "QMessageBox QPushButton {"
                "   background-color: #e74c3c;"
                "   color: white;"
                "   border-radius: 5px;"
                "   padding: 5px 10px;"
                "   min-width: 80px;"
                "}"
                "QMessageBox QPushButton:hover {"
                "   background-color: #c0392b;"
                "}"
                );

            // Mostrar la ventana principal antes del message box
            this->show();

            msgBox.exec();

            if (msgBox.clickedButton() == reintentarButton) {
                on_newGameButton_clicked(); // Llama recursivamente para reiniciar
            }
            // Si elige menuButton, ya estamos en el menú principal
        });

        this->hide();

        // Crear texto del título con estilo Dragon Ball
        QGraphicsTextItem* tituloNivel = new QGraphicsTextItem("Nivel 1: Escape en la Isla");

        // Configurar fuente
        QFont font(dragonBallFont, 24, QFont::Bold);
        if (dragonBallFont == "Arial") {
            font.setLetterSpacing(QFont::AbsoluteSpacing, 2);
        }
        tituloNivel->setFont(font);

        // Color amarillo dorado
        tituloNivel->setDefaultTextColor(QColor(255, 215, 0));

        // Efecto de sombra
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(10);
        shadow->setColor(Qt::black);
        shadow->setOffset(5, 5);
        tituloNivel->setGraphicsEffect(shadow);

        // Centrar el título
        juego->agregarItemEscena(tituloNivel);
        QRectF textRect = tituloNivel->boundingRect();
        tituloNivel->setPos(
            juego->width()/2 - textRect.width()/2,
            juego->height()/2 - textRect.height()/2
            );

        // Temporizador para eliminar el título
        QTimer::singleShot(3000, [this, tituloNivel]() {
            if (juego) {
                juego->removerItemEscena(tituloNivel);
                delete tituloNivel;
                juego->iniciar();
            }
        });

    } catch (const std::exception& e) {
        qCritical() << "Error al crear el juego:" << e.what();
        QMessageBox::critical(this, "Error",
                              tr("No se pudo iniciar el juego.\nDetalles: %1").arg(e.what()));
    } catch (...) {
        qCritical() << "Error desconocido al crear el juego";
        QMessageBox::critical(this, "Error",
                              tr("Error desconocido al iniciar el juego."));
    }
}

MainWindow::~MainWindow() {
    delete ui;
    if (juego) {
        delete juego;
    }
}
