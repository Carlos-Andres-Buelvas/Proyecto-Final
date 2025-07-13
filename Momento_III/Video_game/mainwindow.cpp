#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsTextItem>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

/**
 * @brief Constructor de la ventana principal.
 *
 * Inicializa la interfaz, carga la fuente personalizada,
 * aplica estilos visuales y reproduce la música del menú.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , juego(nullptr)
{
    ui->setupUi(this);

    // Cargar la fuente Dragon Ball
    dragonBallFont = loadDragonBallFont();
    aplicarEstilosMenu();

    // Reproducir música del menú principal
    musicaMenu = new QMediaPlayer(this);
    salidaAudioMenu = new QAudioOutput(this);
    musicaMenu->setSource(QUrl("qrc:/sounds/Sounds/musica_menu.mp3"));
    salidaAudioMenu->setVolume(1.0);
    musicaMenu->setAudioOutput(salidaAudioMenu);
    musicaMenu->setLoops(QMediaPlayer::Infinite);
    musicaMenu->play();
}

/**
 * @brief Carga la fuente personalizada desde los recursos.
 * @return El nombre de la fuente cargada o "Arial" si falla.
 */
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

/**
 * @brief Aplica el estilo visual del menú usando la fuente personalizada.
 */
void MainWindow::aplicarEstilosMenu() {
    if (!ui) return;

    QFont font(dragonBallFont, 18, QFont::Bold);
    ui->newGameButton->setFont(font);
    ui->informationButton->setFont(font);

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
    ui->informationButton->setStyleSheet(style);
}

/**
 * @brief Slot que se ejecuta al hacer clic en el botón "Nuevo Juego".
 */
void MainWindow::on_newGameButton_clicked() {
    if (juego) {
        delete juego;
        juego = nullptr;
    }

    if (musicaMenu && musicaMenu->playbackState() == QMediaPlayer::PlayingState) musicaMenu->stop();

    try {
        juego = new Juego();
        juego->show();

        connect(juego, &Juego::salirAlMenu, this, [this]() {
            juego->hide();
            this->show();
            if (musicaMenu)
                musicaMenu->play();
        });

        connect(juego, &Juego::gameOver, this, [this]() {
            juego->hide();
            mostrarGameOverDialog(false);
        });

        connect(juego, &Juego::nivelCompletado, this, [this]() {
            juego->hide();
            iniciarNivel2();
        });

        this->hide();
        mostrarTituloNivel("Nivel 1: Escape en la Isla", juego);

    } catch (const std::exception& e) {
        qCritical() << "Error al crear el juego:" << e.what();
        QMessageBox::critical(this, "Error", tr("No se pudo iniciar el juego.\nDetalles: %1").arg(e.what()));
    }
}

/**
 * @brief Slot que se ejecuta al hacer clic en el botón "Información".
 */
void MainWindow::on_informationButton_clicked() {
    QDialog* infoDialog = new QDialog(this);
    infoDialog->setWindowTitle("Información del Juego");
    infoDialog->setFixedSize(600, 500);
    infoDialog->setStyleSheet("background-color: #2c3e50;");

    QVBoxLayout* layout = new QVBoxLayout(infoDialog);

    QLabel* infoLabel = new QLabel;
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setOpenExternalLinks(true);  // Esto permite hacer clic en enlaces
    infoLabel->setStyleSheet(R"(
        QLabel {
            color: white;
            font: 14px 'Arial';
        }
        a {
            color: #3498db;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
    )");

    infoLabel->setText(R"(
<b style='font-size:18px;'>PROYECTO FINAL - INFORMATICA II</b><br><br>

<b>🎮 CREADOR:</b> Carlos Andrés Buelvas<br>
<b>🧡 AGRADECIMIENTOS:</b> A los profesores, mis compañeros y... a Goku por aguantar tantos saltos mal calculados.<br><br>

<b>📘 INSTRUCCIONES:</b><br>
<b>Nivel 1 - Escape en la Isla</b><br>
- <b>W</b>: Saltar (mantener para una caída lenta).<br>
- <b>S</b>: Caer más rápido o bajarse de cuerdas/plataformas.<br>
- <b>P</b>: Disparar cuando la energía esté al máximo.<br>
- <b>Espacio</b>: Pausar el juego (¡sólo en este nivel!).<br><br>

<b>Nivel 2 - Rescate de Bulma</b><br>
- <b>W, A, S, D</b>: Mover a Goku por el laberinto.<br>
- <b>P</b>: Disparar hasta 3 proyectiles cuando no estás recargando.<br>
- Evita a los soldados y encuentra las 4 llaves para liberar a Bulma.<br><br>

<b>⚠️ ADVERTENCIAS Y ERRORES:</b><br>
- A veces los soldados tienen puntería de francotirador.<br>
- El botón de "salir al menú" puede cerrar el juego inesperadamente.<br>
- En el nivel 1 las gaviotas y palmeras son solamente decoración, preocupate por las rocas y troncos.<br>
- Si algo falla, ¡respira como Goku cuando entrena con 1000 kilos encima!<br><br>

<b>✨ CONSEJO:</b> Juega con energía, ¡como si estuvieras recargando un Kamehameha!<br><br>

<b>🔗 ENLACES:</b><br>
<a href='https://github.com/Carlos-Andres-Buelvas/Proyecto-Final'>Repositorio en GitHub</a><br>

<center><i>“¡Gracias por jugar! Este proyecto fue hecho con esfuerzo, errores... y mucho café.”</i></center>
)");

    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignTop);

    QPushButton* cerrar = new QPushButton("Cerrar");
    cerrar->setStyleSheet(R"(
        QPushButton {
            background-color: #e74c3c;
            color: white;
            border-radius: 6px;
            padding: 6px 20px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
    )");
    connect(cerrar, &QPushButton::clicked, infoDialog, &QDialog::accept);

    layout->addWidget(infoLabel);
    layout->addWidget(cerrar, 0, Qt::AlignCenter);

    infoDialog->exec();
}

/**
 * @brief Inicia el segundo nivel del juego y configura sus señales.
 */
void MainWindow::iniciarNivel2() {
    Juego2* juego2 = new Juego2();
    juego2->show();

    connect(juego2, &Juego2::salirAlMenu, this, [this, juego2]() {
        juego2->hide();
        this->show();
        if (musicaMenu)
            musicaMenu->play();
    });

    connect(juego2, &Juego2::gameOver, this, [this, juego2]() {
        juego2->hide();
        mostrarGameOverDialog(true);
    });

    mostrarTituloNivel("Nivel 2: Rescate de Bulma", juego2, false);
}

/**
 * @brief Muestra el cuadro de diálogo de Game Over y da opciones al jugador.
 * @param esNivel2 True si corresponde al nivel 2.
 */
void MainWindow::mostrarGameOverDialog(bool esNivel2) {
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
        if (esNivel2) {
            // Reiniciar el nivel 2
            if (juego2) {
                delete juego2;
                juego2 = nullptr;
            }
            iniciarNivel2(); // Usamos la función que ya teníamos
        } else {
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
                mostrarGameOverDialog(false); // false = nivel 1
            });

            connect(juego, &Juego::nivelCompletado, this, &MainWindow::iniciarNivel2);

            this->hide();
            mostrarTituloNivel("Nivel 1: Escape en la Isla", juego);
        }
    }
    else if (msgBox.clickedButton() == menuButton) {
        this->show(); // Solo mostrar menú principal
        if (musicaMenu)
            musicaMenu->play();
    }
}

/**
 * @brief Muestra el título del nivel con efecto visual antes de iniciarlo.
 * @param titulo Texto a mostrar.
 * @param vistaJuego Vista del juego (nivel 1 o 2).
 * @param tieneIniciar Si es true, llama a iniciar() después.
 */
void MainWindow::mostrarTituloNivel(const QString& titulo, QGraphicsView* vistaJuego, bool tieneIniciar) {
    if (!vistaJuego) return;

    QGraphicsTextItem* tituloItem = new QGraphicsTextItem(titulo);
    QFont font(dragonBallFont, 24, QFont::Bold);
    tituloItem->setFont(font);
    tituloItem->setDefaultTextColor(QColor(255, 215, 0));

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(Qt::black);
    shadow->setOffset(5, 5);
    tituloItem->setGraphicsEffect(shadow);

    if (auto juegoPtr = dynamic_cast<Juego*>(vistaJuego)) {
        juegoPtr->agregarItemEscena(tituloItem);
    } else if (auto juego2Ptr = dynamic_cast<Juego2*>(vistaJuego)) {
        juego2Ptr->agregarItemEscena(tituloItem);
    }

    QRectF textRect = tituloItem->boundingRect();
    tituloItem->setPos(
        vistaJuego->width()/2 - textRect.width()/2,
        vistaJuego->height()/2 - textRect.height()/2
        );

    QTimer::singleShot(3000, [this, vistaJuego, tituloItem, tieneIniciar]() {
        if (auto juegoPtr = dynamic_cast<Juego*>(vistaJuego)) {
            juegoPtr->removerItemEscena(tituloItem);
            if (tieneIniciar) juegoPtr->iniciar();
        } else if (auto juego2Ptr = dynamic_cast<Juego2*>(vistaJuego)) {
            juego2Ptr->removerItemEscena(tituloItem);
        }
        delete tituloItem;
    });
}

/**
 * @brief Destructor de la ventana principal.
 */
MainWindow::~MainWindow() {
    delete ui;
    if (juego) delete juego;
    if (musicaMenu) delete musicaMenu;
    if (salidaAudioMenu) delete salidaAudioMenu;
}
