#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QGraphicsPixmapItem>
#include <QTimer>

// Clase base abstracta para personajes jugables o enemigos
class Personaje : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    // Constructor base
    Personaje(float x, float y, float ancho, float alto);

    // Métodos virtuales puros para comportamiento específico
    virtual void mover() = 0;
    virtual void disparar(QGraphicsScene* escena) = 0;

//NIVEL 2:

    // Nuevos métodos para nivel 2 (cenital)
    void mover2(const QString& direccion);
    void disparar2(QGraphicsScene* escena);
    bool colisionaConMuro(const QPointF& nuevaPos, QGraphicsScene* escena);

signals:
    void actualizarBarraEnergiaSignal();
    void gokuDerrotado();

protected:
    // Atributos físicos y visuales del personaje
    float posX, posY;       // Posición
    float velocidadY;       // Velocidad vertical
    float gravedad;         // Intensidad de la caída
    float ancho, alto;      // Dimensiones

    int frameActual = 0;
    QVector<QPixmap> framesArriba;
    QVector<QPixmap> framesAbajo;
    QVector<QPixmap> framesIzquierda;
    QVector<QPixmap> framesDerecha;

    //QVector<QGraphicsItem*> proyectilesActivos;
    QVector<QPair<QGraphicsEllipseItem*, QTimer*>> proyectilesActivos;
    QString tipo;
};

#endif // PERSONAJE_H
