#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsTextItem>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , juego(nullptr)
    , juego2(nullptr)
{
    ui->setupUi(this);
    setFixedSize(800, 600);  // ⏹️ Tamaño fijo SOLO para el menú principal
    // Centrar la ventana principal (menú)
    QTimer::singleShot(0, this, [this]() {
        QScreen* screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - this->width()) / 2;
        int y = (screenGeometry.height() - this->height()) / 2;
        this->move(x, y);
    });

    // Cargar fuente Dragon Ball
    dragonBallFont = loadDragonBallFont();
    aplicarEstilosMenu();
}

QString MainWindow::loadDragonBallFont() {
    QString fontPath = ":/fondos/Pictures/db_font.ttf";

    if (!QFile::exists(fontPath)) {
        qDebug() << "Error: No se encontró la fuente en" << fontPath;
        return "Arial";
    }

    int fontId = QFontDatabase::addApplicationFont(fontPath);
    if (fontId == -1) {
        qDebug() << "Error al cargar fuente Dragon Ball";
        return "Arial";
    }

    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    return fontFamilies.isEmpty() ? "Arial" : fontFamilies.first();
}

void MainWindow::aplicarEstilosMenu() {
    if (!ui) return;  // Evita errores si UI no está inicializado

    QFont font(dragonBallFont, 18, QFont::Bold);
    ui->newGameButton->setFont(font);
    ui->continueButton->setFont(font);

    QString style = R"(
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
    ui->newGameButton->setStyleSheet(style);
    ui->continueButton->setStyleSheet(style);
}

void MainWindow::on_newGameButton_clicked() {
    if (juego) {
        delete juego;
        juego = nullptr;
    }

    try {
        juego = new Juego(this);
        setCentralWidget(juego);
        resize(1280, 680);
        setFixedSize(1280, 680);

        // ✅ Centrar justo después de mostrar (en el próximo "tick" del evento loop)
        QTimer::singleShot(0, this, [this]() {
            QScreen* screen = QGuiApplication::primaryScreen();
            QRect screenGeometry = screen->availableGeometry();  // Usa área disponible (excluye barra de tareas)
            int x = (screenGeometry.width() - this->width()) / 2;
            int y = (screenGeometry.height() - this->height()) / 2;
            this->move(x, y);
        });

        setFixedSize(1280, 680);     // Evita redimensionar durante el juego
        juego->setFocus();

        // Conectar señales
        connect(juego, &Juego::salirAlMenu, this, &MainWindow::volverAlMenu);
        connect(juego, &Juego::nivelCompletado, this, &MainWindow::iniciarNivel2);

        connect(juego, &Juego::gameOver, this, [this]() {
            juego->hide();

            QMessageBox msgBox;
            msgBox.setWindowTitle("Game Over");
            msgBox.setText("¡Has perdido! ¿Qué quieres hacer?");

            QPushButton *reintentar = msgBox.addButton("Reintentar", QMessageBox::AcceptRole);
            QPushButton *menu = msgBox.addButton("Menú Principal", QMessageBox::RejectRole);
            msgBox.setDefaultButton(reintentar);

            msgBox.setStyleSheet(R"(
                QMessageBox {
                    background-color: #2c3e50;
                    color: white;
                }
                QMessageBox QLabel {
                    color: white;
                    font: bold 16px;
                }
                QMessageBox QPushButton {
                    background-color: #e74c3c;
                    color: white;
                    border-radius: 5px;
                    padding: 5px 10px;
                    min-width: 80px;
                }
                QMessageBox QPushButton:hover {
                    background-color: #c0392b;
                }
            )");

            msgBox.exec();

            if (msgBox.clickedButton() == reintentar) {
                on_newGameButton_clicked();
            } else {
                volverAlMenu();
            }
        });

        // Título de nivel 1
        QGraphicsTextItem* titulo = new QGraphicsTextItem("Nivel 1: Escape en la Isla");
        QFont font(dragonBallFont, 24, QFont::Bold);
        if (dragonBallFont == "Arial")
            font.setLetterSpacing(QFont::AbsoluteSpacing, 2);

        titulo->setFont(font);
        titulo->setDefaultTextColor(QColor(255, 215, 0));
        QGraphicsDropShadowEffect* sombra = new QGraphicsDropShadowEffect();
        sombra->setBlurRadius(10);
        sombra->setColor(Qt::black);
        sombra->setOffset(5, 5);
        titulo->setGraphicsEffect(sombra);
        juego->agregarItemEscena(titulo);

        QRectF rect = titulo->boundingRect();
        titulo->setPos(juego->width()/2 - rect.width()/2, juego->height()/2 - rect.height()/2);

        QTimer::singleShot(3000, [this, titulo]() {
            if (juego) {
                juego->removerItemEscena(titulo);
                delete titulo;
                juego->iniciar();
            }
        });

    } catch (...) {
        qCritical() << "Error desconocido al iniciar el juego";
        QMessageBox::critical(this, "Error", "No se pudo iniciar el juego.");
    }
}

void MainWindow::iniciarNivel2() {
    if (juego2) {
        delete juego2;
        juego2 = nullptr;
    }

    try {
        juego2 = new Juego2(this);
        setCentralWidget(juego2);
        resize(1280, 680);
        setFixedSize(1280, 680);
        juego2->setFocus();

        juego2->mostrarTituloNivel();

        disconnect(juego2, nullptr, this, nullptr);  // 👈 Detiene conexiones anteriores

        connect(juego2, &Juego2::gameOver, this, [this]() {
            if (!juego2) return;  // Por seguridad

            juego2->hide();

            static bool mostrado = false;
            if (mostrado) return;
            mostrado = true;

            QMessageBox msgBox;
            msgBox.setWindowTitle("Game Over");
            msgBox.setText("¡Has perdido! ¿Qué quieres hacer?");

            QPushButton *reintentar = msgBox.addButton("Reintentar", QMessageBox::AcceptRole);
            QPushButton *menu = msgBox.addButton("Menú Principal", QMessageBox::RejectRole);
            msgBox.setDefaultButton(reintentar);

            msgBox.setStyleSheet(R"(
        QMessageBox {
            background-color: #2c3e50;
            color: white;
        }
        QMessageBox QLabel {
            color: white;
            font: bold 16px;
        }
        QMessageBox QPushButton {
            background-color: #e74c3c;
            color: white;
            border-radius: 5px;
            padding: 5px 10px;
            min-width: 80px;
        }
        QMessageBox QPushButton:hover {
            background-color: #c0392b;
        }
    )");

            msgBox.exec();
            mostrado = false;

            if (msgBox.clickedButton() == reintentar) {
                iniciarNivel2();
            } else {
                volverAlMenu();
            }
        });

    } catch (...) {
        qCritical() << "Error desconocido al iniciar el nivel 2";
        QMessageBox::critical(this, "Error", "No se pudo iniciar el nivel 2.");
    }
}

void MainWindow::volverAlMenu() {
    setCentralWidget(nullptr);

    if (juego) {
        delete juego;
        juego = nullptr;
    }

    if (juego2) {
        delete juego2;
        juego2 = nullptr;
    }

    // Vuelve al menú principal
    ui->setupUi(this);          // Recarga interfaz original
    aplicarEstilosMenu();       // Aplica fuente y estilo
    resize(800, 600);           // Vuelve a tamaño original del menú
    setFixedSize(800, 600);
    this->show();
}

MainWindow::~MainWindow() {
    delete ui;
    delete juego;
    delete juego2;
}
