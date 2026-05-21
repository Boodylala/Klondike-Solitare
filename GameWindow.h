#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QPushButton>
#include "KlondikeSolitaire.h"

class CardItem;

class GameWindow : public QMainWindow {
    Q_OBJECT

private:
    KlondikeSolitaire* game;
    QGraphicsScene* scene;
    QGraphicsView* view;
    QLabel* scoreLabel;
    QPushButton* newGameButton;
    QPushButton* undoButton;

    static const int CARD_WIDTH = 80;
    static const int CARD_HEIGHT = 120;
    static const int PILE_SPACING = 100;
    static const int VERTICAL_OFFSET = 30;
    static const int SCENE_WIDTH = 850;
    static const int SCENE_HEIGHT = 800;

    QPointF stockPos;
    QPointF wastePos;
    QPointF foundationPos[4];
    QPointF tableauPos[7];

    CardItem* draggedCard;
    QPointF dragStartPos;
    int dragSourceType;
    int dragSourceIndex;
    int dragCardIndex;

    void setupUI();
    void calculatePositions();
    void updateDisplay();
    void clearScene();
    CardItem* createPlaceholder(QPointF pos, QString text);

public:
    GameWindow(QWidget* parent = nullptr);
    ~GameWindow();

    void startDrag(CardItem* card, int sourceType, int sourceIndex, int cardIndex = -1);
    void endDrag(CardItem* card, QPointF dropPos);
    bool tryDropOnTableau(int tableauIndex, Card* card);
    bool tryDropOnFoundation(int foundIndex, Card* card);
    void onStockClicked();

private slots:
    void onNewGame();
    void onUndo();
};

class CardItem : public QGraphicsPixmapItem {
private:
    Card* cardData;
    GameWindow* gameWindow;
    int sourceType;
    int sourceIndex;
    int cardIndex;
    bool isDraggable;
    QPointF originalPos;
    bool isBeingDragged;

public:
    CardItem(Card* card, GameWindow* window, int srcType, int srcIndex, QPixmap pixmap, int cardIdx = -1);

    Card* getCard() { return cardData; }
    int getSourceType() { return sourceType; }
    int getSourceIndex() { return sourceIndex; }
    int getCardIndex() { return cardIndex; }
    void setDraggable(bool drag) { isDraggable = drag; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif
