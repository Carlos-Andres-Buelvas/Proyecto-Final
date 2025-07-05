#ifndef GOKU_H
#define GOKU_H

#include "personaje.h"
#include <QVector>
#include <QPixmap>
#include <QGraphicsScene>

class Enemigo;

class Goku : public Personaje {
    Q_OBJECT

public:
    Goku(float x, float y, float ancho, float alto);

    // Comportamientos principales
    void mover() override;
    void disparar(QGraphicsScene* escena) override;
    void saltar();
    void acelerarCaida();             // Acción al presionar tecla S

    // Animaciones
    void animarCorrer();
    void animarCaida();
    void animarDisparo();
    void animarCuerda();
    void animarDisparoFrame();       // Se llama cada 200ms al disparar

    // Estado de disparo y energía
    bool estaDisparando() const;
    bool puedeDisparar() const { return energia >= energiaMaxima; }
    void reiniciarEnergia() { energia = 0; }
    void aumentarEnergia(int cantidad) {
        energia += cantidad;
        if (energia > energiaMaxima) energia = energiaMaxima;
    }
    int obtenerEnergia() const { return energia; }

    // Estado físico
    bool estaBajando() const;
    bool estaEnSuelo() const;
    void detenerCaida();
    void activarCaida();
    void forzarCaida();
    bool estaForzandoCaida() const;
    void cancelarCaidaForzada();

    // Referencia a enemigos para interacción
    void setListaEnemigos(QVector<Enemigo*>* lista);

    // Estado de entrada
    bool teclaWSostenida = false;    // Si W está presionada
    bool cayendoLento = false;       // Control de caída suave

private:
    // Animaciones
    QVector<QPixmap> framesCorrer;
    QVector<QPixmap> framesCaer;
    QVector<QPixmap> framesDisparo;
    QPixmap spriteCuerda;

    int frameActual;
    int contador;
    int velocidadAnimacion;

    // Disparo
    bool disparando;
    QTimer* disparoTimer;
    int frameDisparoActual;

    // Estado en el suelo
    bool enSuelo;

    // Energía
    int energia = 0;
    const int energiaMaxima = 100;

    // Enemigos cercanos
    QVector<Enemigo*>* listaEnemigos = nullptr;

    // Control de caída manual (por tecla S)
    bool forzarCaidaManual = false;

    // Métodos internos
    void cargarAnimaciones();        // Carga los sprites
    void actualizarSprite();         // Aplica el frame actual
};

#endif // GOKU_H
