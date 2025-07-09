#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsTextItem>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>  // Mostrar mensajes al usuario

// Constructor de MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , juego(nullptr)
{
    ui->setupUi(this);

    // Cargar la fuente Dragon Ball
    dragonBallFont = loadDragonBallFont();

    // Aplicar fuente personalizada a los botones
    QFont font(dragonBallFont, 18, QFont::Bold);
    ui->newGameButton->setFont(font);
    ui->continueButton->setFont(font);

    // Estilo visual para botones
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

// Carga la fuente personalizada desde el recurso
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

// Slot al hacer clic en "Nuevo Juego"
void MainWindow::on_newGameButton_clicked() {
    if (juego) {
        delete juego;
        juego = nullptr;
    }

    try {
        juego = new Juego();
        juego->show();

        // Conectar señal para volver al menú principal
        connect(juego, &Juego::salirAlMenu, this, [this]() {
            juego->hide();
            this->show();
        });

        // Conectar señal de Game Over
        connect(juego, &Juego::gameOver, this, [this]() {
            juego->hide();
            mostrarGameOverDialog();
        });

        // Conectar señal de nivel completado
        connect(juego, &Juego::nivelCompletado, this, [this]() {
            juego->hide();
            iniciarNivel2();
        });

        this->hide();

        // Mostrar título del nivel
        mostrarTituloNivel("Nivel 1: Escape en la Isla");

    } catch (const std::exception& e) {
        qCritical() << "Error al crear el juego:" << e.what();
        QMessageBox::critical(this, "Error", tr("No se pudo iniciar el juego.\nDetalles: %1").arg(e.what()));
    }
}

void MainWindow::iniciarNivel2() {
    Juego2* juego2 = new Juego2();
    juego2->show();

    // Conectar señal para volver al menú principal desde nivel 2
    connect(juego2, &Juego2::salirAlMenu, this, [this, juego2]() {
        juego2->hide();
        this->show();
        //delete juego2;
    });

    // Mostrar título del nivel 2
    QGraphicsTextItem* tituloNivel = new QGraphicsTextItem("Nivel 2: Rescate de Bulma");
    QFont font(dragonBallFont, 24, QFont::Bold);
    tituloNivel->setFont(font);
    tituloNivel->setDefaultTextColor(QColor(255, 215, 0));

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(Qt::black);
    shadow->setOffset(5, 5);
    tituloNivel->setGraphicsEffect(shadow);

    juego2->agregarItemEscena(tituloNivel);
    QRectF textRect = tituloNivel->boundingRect();
    tituloNivel->setPos(
        juego2->width()/2 - textRect.width()/2,
        juego2->height()/2 - textRect.height()/2
        );

    // Quitar el título después de 3 segundos
    QTimer::singleShot(3000, [juego2, tituloNivel]() {
        juego2->removerItemEscena(tituloNivel);
        delete tituloNivel;
    });
}

void MainWindow::mostrarGameOverDialog() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Game Over");
    msgBox.setText("¡Has perdido! ¿Qué quieres hacer?");

    QPushButton *reintentarButton = msgBox.addButton("Reintentar", QMessageBox::ActionRole);
    QPushButton *menuButton = msgBox.addButton("Menú Principal", QMessageBox::ActionRole);
    msgBox.setDefaultButton(reintentarButton);

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

    int result = msgBox.exec();

    if (msgBox.clickedButton() == reintentarButton) {
        // Reiniciar el nivel 1
        if (juego) {
            delete juego;
            juego = nullptr;
        }
        juego = new Juego();
        juego->show();

        // Reconectar señales
        connect(juego, &Juego::salirAlMenu, this, [this]() {
            juego->hide();
            this->show();
        });

        connect(juego, &Juego::gameOver, this, [this]() {
            juego->hide();
            mostrarGameOverDialog();
        });

        connect(juego, &Juego::nivelCompletado, this, &MainWindow::iniciarNivel2);

        this->hide();
        mostrarTituloNivel("Nivel 1: Escape en la Isla");
    }
    else if (msgBox.clickedButton() == menuButton) {
        this->show(); // Solo mostrar menú principal
    }
}

void MainWindow::mostrarTituloNivel(const QString& titulo) {
    QGraphicsTextItem* tituloItem = new QGraphicsTextItem(titulo);
    QFont font(dragonBallFont, 24, QFont::Bold);
    tituloItem->setFont(font);
    tituloItem->setDefaultTextColor(QColor(255, 215, 0));

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(Qt::black);
    shadow->setOffset(5, 5);
    tituloItem->setGraphicsEffect(shadow);

    juego->agregarItemEscena(tituloItem);
    QRectF textRect = tituloItem->boundingRect();
    tituloItem->setPos(
        juego->width()/2 - textRect.width()/2,
        juego->height()/2 - textRect.height()/2
        );

    QTimer::singleShot(3000, [this, tituloItem]() {
        juego->removerItemEscena(tituloItem);
        delete tituloItem;
        juego->iniciar();
    });
}

// Destructor de MainWindow
MainWindow::~MainWindow() {
    delete ui;
    if (juego) {
        delete juego;
    }
}
