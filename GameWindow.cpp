#include "GameWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QFont>
#include <QApplication>
#include <QScreen>

QPixmap createCardPixmap(Card* card, int width, int height) {
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRoundedRect(2, 2, width-4, height-4, 5, 5);

    if (!card->isFaceUp()) {
        painter.setBrush(QBrush(QColor(0, 0, 150)));
        painter.drawRoundedRect(5, 5, width-10, height-10, 5, 5);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawLine(10, 10, width-10, height-10);
        painter.drawLine(width-10, 10, 10, height-10);
    } else {
        QString rankStr;
        switch(card->getRank()) {
        case Card::ACE: rankStr = "A"; break;
        case Card::JACK: rankStr = "J"; break;
        case Card::QUEEN: rankStr = "Q"; break;
        case Card::KING: rankStr = "K"; break;
        default: rankStr = QString::number(card->getRank()); break;
        }

        QString suitStr;
        QColor color;
        switch(card->getSuit()) {
        case Card::HEARTS: suitStr = "♥"; color = Qt::red; break;
        case Card::DIAMONDS: suitStr = "♦"; color = Qt::red; break;
        case Card::CLUBS: suitStr = "♣"; color = Qt::black; break;
        case Card::SPADES: suitStr = "♠"; color = Qt::black; break;
        }

        painter.setPen(color);

        QFont font("Arial", 16, QFont::Bold);
        painter.setFont(font);
        painter.drawText(10, 25, rankStr);

        QFont suitFont("Arial", 20, QFont::Bold);
        painter.setFont(suitFont);
        painter.drawText(10, 50, suitStr);

        QFont centerFont("Arial", 40, QFont::Bold);
        painter.setFont(centerFont);
        painter.drawText(QRect(0, 0, width, height), Qt::AlignCenter, suitStr);

        painter.save();
        painter.translate(width, height);
        painter.rotate(180);
        painter.setFont(font);
        painter.drawText(10, 25, rankStr);
        painter.setFont(suitFont);
        painter.drawText(10, 50, suitStr);
        painter.restore();
    }

    return pixmap;
}

GameWindow::GameWindow(QWidget* parent) : QMainWindow(parent) {
    game = new KlondikeSolitaire();
    draggedCard = nullptr;
    dragCardIndex = -1;

    setupUI();
    calculatePositions();
    updateDisplay();
}

GameWindow::~GameWindow() {
    delete game;
}

void GameWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->setContentsMargins(10, 10, 10, 10);

    scoreLabel = new QLabel("Score: 0", this);
    scoreLabel->setFont(QFont("Arial", 14, QFont::Bold));
    scoreLabel->setStyleSheet("color: white; background-color: rgba(0, 50, 0, 180); padding: 5px 10px; border-radius: 5px;");

    newGameButton = new QPushButton("New Game", this);
    undoButton = new QPushButton("Undo", this);

    newGameButton->setStyleSheet("padding: 5px 15px;");
    undoButton->setStyleSheet("padding: 5px 15px;");

    connect(newGameButton, &QPushButton::clicked, this, &GameWindow::onNewGame);
    connect(undoButton, &QPushButton::clicked, this, &GameWindow::onUndo);

    controlLayout->addWidget(scoreLabel);
    controlLayout->addStretch();
    controlLayout->addWidget(undoButton);
    controlLayout->addWidget(newGameButton);

    mainLayout->addLayout(controlLayout);

    // Create scene with fixed size
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, SCENE_WIDTH, SCENE_HEIGHT);

    view = new QGraphicsView(scene, this);
    view->setFixedSize(SCENE_WIDTH + 2, SCENE_HEIGHT + 2);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setBackgroundBrush(QBrush(QColor(0, 100, 0)));
    view->setFrameStyle(QFrame::Box | QFrame::Plain);
    view->setLineWidth(2);

    // Add decorative border
    QPalette palette = view->palette();
    palette.setColor(QPalette::Base, QColor(0, 100, 0));
    view->setPalette(palette);

    mainLayout->addWidget(view, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    setCentralWidget(centralWidget);
    setWindowTitle("Klondike Solitaire");

    // Set fixed window size
    int windowWidth = SCENE_WIDTH + 20;
    int windowHeight = SCENE_HEIGHT + 100;
    setFixedSize(windowWidth, windowHeight);

    // Center window on screen
    // Center window on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - windowWidth) / 2;
    int y = (screenGeometry.height() - windowHeight) / 2;
    move(x, y);
}

void GameWindow::calculatePositions() {
    stockPos = QPointF(50, 50);
    wastePos = QPointF(150, 50);

    for (int i = 0; i < 4; i++) {
        foundationPos[i] = QPointF(450 + i * PILE_SPACING, 50);
    }

    for (int i = 0; i < 7; i++) {
        tableauPos[i] = QPointF(50 + i * PILE_SPACING, 220);
    }
}

CardItem* GameWindow::createPlaceholder(QPointF pos, QString text) {
    QPixmap pixmap(CARD_WIDTH, CARD_HEIGHT);
    pixmap.fill(QColor(0, 80, 0, 100));

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
    painter.drawRoundedRect(2, 2, CARD_WIDTH-4, CARD_HEIGHT-4, 5, 5);

    if (!text.isEmpty()) {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(QRect(0, 0, CARD_WIDTH, CARD_HEIGHT), Qt::AlignCenter, text);
    }

    CardItem* item = new CardItem(nullptr, this, -2, -1, pixmap, -1);
    item->setPos(pos);
    item->setDraggable(false);
    item->setZValue(0);
    scene->addItem(item);

    return item;
}

void GameWindow::updateDisplay() {
    clearScene();

    static Card stockBackCard(Card::HEARTS, Card::ACE);
    stockBackCard.setFaceUp(false);

    if (game->isStockEmpty()) {
        QPixmap pixmap(CARD_WIDTH, CARD_HEIGHT);
        pixmap.fill(QColor(0, 80, 0, 100));

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
        painter.drawRoundedRect(2, 2, CARD_WIDTH-4, CARD_HEIGHT-4, 5, 5);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(QRect(0, 0, CARD_WIDTH, CARD_HEIGHT), Qt::AlignCenter, "Empty\n(Click to\nRefill)");

        CardItem* emptyStock = new CardItem(&stockBackCard, this, -1, -1, pixmap, -1);
        emptyStock->setPos(stockPos);
        emptyStock->setDraggable(false);
        emptyStock->setZValue(1);
        scene->addItem(emptyStock);
    } else {
        QPixmap stockPixmap = createCardPixmap(&stockBackCard, CARD_WIDTH, CARD_HEIGHT);
        CardItem* stockItem = new CardItem(&stockBackCard, this, -1, -1, stockPixmap, -1);
        stockItem->setPos(stockPos);
        stockItem->setDraggable(false);
        stockItem->setZValue(1);
        scene->addItem(stockItem);
    }

    Card* wasteCard = game->getWasteTop();
    if (wasteCard) {
        QPixmap wastePixmap = createCardPixmap(wasteCard, CARD_WIDTH, CARD_HEIGHT);
        CardItem* wasteItem = new CardItem(wasteCard, this, 0, 0, wastePixmap, -1);
        wasteItem->setPos(wastePos);
        wasteItem->setDraggable(true);
        wasteItem->setZValue(10);
        scene->addItem(wasteItem);
    } else {
        createPlaceholder(wastePos, "Waste");
    }

    for (int i = 0; i < 4; i++) {
        Card* foundCard = game->getFoundationTop(i);

        if (foundCard) {
            QPixmap foundPixmap = createCardPixmap(foundCard, CARD_WIDTH, CARD_HEIGHT);
            CardItem* item = new CardItem(foundCard, this, 2, i, foundPixmap, -1);
            item->setPos(foundationPos[i]);
            item->setDraggable(true);
            item->setZValue(5);
            scene->addItem(item);
        } else {
            QString label;
            switch(i) {
            case 0: label = "♥"; break;
            case 1: label = "♦"; break;
            case 2: label = "♣"; break;
            case 3: label = "♠"; break;
            }
            createPlaceholder(foundationPos[i], label);
        }
    }

    for (int i = 0; i < 7; i++) {
        int cardCount = game->getTableauSize(i);

        if (cardCount == 0) {
            createPlaceholder(tableauPos[i], "K");
        } else {
            for (int j = 0; j < cardCount; j++) {
                Card* card = game->getTableauCard(i, j);
                QPixmap cardPixmap = createCardPixmap(card, CARD_WIDTH, CARD_HEIGHT);
                CardItem* item = new CardItem(card, this, 1, i, cardPixmap, j);

                QPointF cardPos = tableauPos[i] + QPointF(0, j * VERTICAL_OFFSET);
                item->setPos(cardPos);
                item->setDraggable(card->isFaceUp());
                item->setZValue(j + 1);
                scene->addItem(item);
            }
        }
    }

    scoreLabel->setText(QString("Score: %1").arg(game->getScore()));

    // Update undo button state
    undoButton->setEnabled(game->canUndo());

    if (game->isGameWon()) {
        QMessageBox::information(this, "Congratulations!",
                                 QString("You won!\nFinal Score: %1").arg(game->getScore()));
    }
}

void GameWindow::clearScene() {
    scene->clear();
}

void GameWindow::onNewGame() {
    delete game;
    game = new KlondikeSolitaire();
    updateDisplay();
}

void GameWindow::onUndo() {
    if (game->undo()) {
        updateDisplay();
    } else {
        QMessageBox::information(this, "Undo", "Nothing to undo!");
    }
}

void GameWindow::onStockClicked() {
    game->drawFromStock();
    updateDisplay();
}

void GameWindow::startDrag(CardItem* card, int sourceType, int sourceIndex, int cardIndex) {
    draggedCard = card;
    dragSourceType = sourceType;
    dragSourceIndex = sourceIndex;
    dragCardIndex = cardIndex;
    dragStartPos = card->pos();

    // If dragging from tableau, raise all cards in the sequence
    if (sourceType == 1 && cardIndex >= 0) {
        int cardCount = game->getTableauSize(sourceIndex);
        QList<QGraphicsItem*> items = scene->items();

        for (QGraphicsItem* item : items) {
            CardItem* cardItem = dynamic_cast<CardItem*>(item);
            if (cardItem && cardItem->getSourceType() == 1 &&
                cardItem->getSourceIndex() == sourceIndex) {
                int idx = cardItem->getCardIndex();
                if (idx >= cardIndex) {
                    cardItem->setZValue(100 + (idx - cardIndex));
                }
            }
        }
    } else {
        card->setZValue(100);
    }
}

void GameWindow::endDrag(CardItem* card, QPointF dropPos) {
    bool validMove = false;

    for (int i = 0; i < 4; i++) {
        QRectF foundRect(foundationPos[i].x() - 20, foundationPos[i].y() - 20,
                         CARD_WIDTH + 40, CARD_HEIGHT + 40);
        if (foundRect.contains(dropPos)) {
            validMove = tryDropOnFoundation(i, card->getCard());
            if (validMove) break;
        }
    }

    if (!validMove) {
        for (int i = 0; i < 7; i++) {
            QRectF tabRect(tableauPos[i].x() - 20, tableauPos[i].y() - 20,
                           CARD_WIDTH + 40, CARD_HEIGHT + 400);
            if (tabRect.contains(dropPos)) {
                validMove = tryDropOnTableau(i, card->getCard());
                if (validMove) break;
            }
        }
    }

    if (validMove) {
        updateDisplay();
    } else {
        card->setPos(dragStartPos);
        card->setZValue(10);
    }

    draggedCard = nullptr;
}

bool GameWindow::tryDropOnTableau(int tableauIndex, Card* card) {
    if (dragSourceType == 0) {
        return game->moveWasteToTableau(tableauIndex);
    } else if (dragSourceType == 1) {
        int sourceSize = game->getTableauSize(dragSourceIndex);
        if (dragCardIndex >= 0 && dragCardIndex < sourceSize - 1) {
            return game->moveTableauSequence(dragSourceIndex, tableauIndex, dragCardIndex);
        } else {
            return game->moveTableauToTableau(dragSourceIndex, tableauIndex);
        }
    } else if (dragSourceType == 2) {
        return game->moveFoundationToTableau(dragSourceIndex, tableauIndex);
    }
    return false;
}

bool GameWindow::tryDropOnFoundation(int foundIndex, Card* card) {
    if (dragSourceType == 0) {
        return game->moveWasteToFoundation(foundIndex);
    } else if (dragSourceType == 1) {
        return game->moveTableauToFoundation(dragSourceIndex, foundIndex);
    }
    return false;
}

CardItem::CardItem(Card* card, GameWindow* window, int srcType, int srcIndex, QPixmap pixmap, int cardIdx)
    : QGraphicsPixmapItem(pixmap), cardData(card), gameWindow(window),
    sourceType(srcType), sourceIndex(srcIndex), cardIndex(cardIdx),
    isDraggable(false), isBeingDragged(false) {
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (isDraggable && cardData != nullptr) {
            originalPos = pos();
            isBeingDragged = true;
            gameWindow->startDrag(this, sourceType, sourceIndex, cardIndex);
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        } else if (sourceType == -1 && sourceIndex == -1 && cardData != nullptr) {
            gameWindow->onStockClicked();
            event->accept();
            return;
        }
    }
    QGraphicsPixmapItem::mousePressEvent(event);
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (isBeingDragged && isDraggable && (event->buttons() & Qt::LeftButton)) {
        QPointF newPos = event->scenePos() - QPointF(40, 60);
        setPos(newPos);
        event->accept();
        return;
    }
    QGraphicsPixmapItem::mouseMoveEvent(event);
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (isBeingDragged && isDraggable && event->button() == Qt::LeftButton) {
        setCursor(Qt::ArrowCursor);
        isBeingDragged = false;
        gameWindow->endDrag(this, event->scenePos());
        event->accept();
        return;
    }
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}
