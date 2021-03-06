#include "viewer.h"
#include "ui_viewer.h"

Viewer::Viewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Viewer)
{
    ui->setupUi(this);
    QPixmap p = QPixmap(ui->animateLabel->size());
    p.fill(QColor(Qt::black));
    ui->animateLabel->setPixmap(p);
    ui->listWidget->setIconSize(QSize(50,50));

    addItemToFrameList();

    connect(ui->moveDown, &QPushButton::released, this, &Viewer::on_moveDowButton_Clicked);
    connect(ui->moveUp, &QPushButton::released, this, &Viewer::on_moveUpButton_Clicked);
    connect(ui->delteFrame, &QPushButton::released, this, &Viewer::on_deleteFrameButton_Clicked);
    connect(ui->addFrame, &QPushButton::released, this, &Viewer::on_addFrameButton_Clicked);
    connect(ui->playButton, &QPushButton::released, this, [this](){
        emit startPlayback(true);});
    connect(ui->playActualSizeButton, &QPushButton::released, this, [this](){
        emit startPlayback(true);});
    connect(ui->pauseBotton, &QPushButton::released, this, [this](){
        emit startPlayback(false);});
    connect(ui->listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem * item){
        emit setEditingFrame(item->data(0).toInt());});
    connect(ui->pencilButton, &QPushButton::released, this, [this](){
        emit switchToolTo(0);});
    connect(ui->eraserButton, &QPushButton::released, this, [this](){
        emit switchToolTo(1);});
    connect(ui->colorPickerButton, &QPushButton::released, this, [this](){
        emit switchToolTo(2);});
    connect(ui->bucketButton, &QPushButton::released, this, [this](){
        emit switchToolTo(3);});
    connect(ui->FPSSlider, &QSlider::valueChanged, this, &Viewer::onSliderValueChangedSlot);
}

Viewer::~Viewer()
{
    delete ui;
}

void Viewer::playback(const QImage &frameImage){
    QPixmap p;
    if(actualSize){
        p = QPixmap::fromImage(frameImage);
    }else{
        p = QPixmap::fromImage(frameImage.scaled(ui->animateLabel->size(), Qt::KeepAspectRatio));
    }

    ui->animateLabel->setPixmap(p);
}
void Viewer::updateEditor(const QImage &frameImage, int editingTarget){
    image = frameImage;
    QPixmap p = QPixmap::fromImage(frameImage.scaled(QSize(50, 50), Qt::KeepAspectRatio));
    frameList[editingTarget]->setIcon(QIcon(p));
    update();
    changed = true;

}
void Viewer::saveCallback(bool success){
    if(success)
        changed = false;
}
void Viewer::loadCallback(bool success){
    if(success)
        repaint();
}
void Viewer::mousePressEvent(QMouseEvent * event){
    if(event->button() == Qt::LeftButton){//if mouse left button clicked
        QPoint screenPos = event -> pos();
        pixelPos = QPoint((screenPos.x()-drawingPivot.x())/(pixelSize+pixelOffset), (screenPos.y()-drawingPivot.y())/(pixelSize+pixelOffset));
        emit useToolOn(pixelPos);//get mouse position as start point
    }else if(event->button() == Qt::RightButton){
        movePivot = event->pos();
    }

}

void Viewer::mouseMoveEvent(QMouseEvent * event){
    if(event->buttons()&Qt::LeftButton){//if mouse left button clicked and move at same time
        QPoint screenPos = event -> pos();
        if(pixelPos != QPoint((screenPos.x()-drawingPivot.x())/(pixelSize+pixelOffset), (screenPos.y()-drawingPivot.y())/(pixelSize+pixelOffset))){
            pixelPos = QPoint((screenPos.x()-drawingPivot.x())/(pixelSize+pixelOffset), (screenPos.y()-drawingPivot.y())/(pixelSize+pixelOffset));
            emit useToolOn(pixelPos);
        }
    }else{
        drawingPivot += event->pos() - movePivot;
        movePivot = event->pos();
        repaint();
    }


}
void Viewer::wheelEvent(QWheelEvent * event){
    //scale function
    pixelSize += event->angleDelta().y()/120;
    if(pixelSize<15){
        pixelOffset = 0;
    }else{
        pixelOffset = 1;
    }
    repaint();
}
void Viewer::paintEvent(QPaintEvent *){
    QPainter painter(this);
    int pos = pixelSize+pixelOffset;
    for(int i = 0; i < image.width(); i++){
        for(int j = 0; j < image.height(); j++){
            painter.fillRect(i * pos + drawingPivot.x(), j * pos + drawingPivot.y(), pixelSize, pixelSize, image.pixelColor(i, j));
        }
    }
}
void Viewer::on_moveUpButton_Clicked(){
    int id = ui->listWidget->currentItem()->data(0).toInt();
    if(id == 0)
        return;
    emit moveFrame(id-1, id);
    ui->listWidget->setCurrentRow(id-1);
    emit setEditingFrame(id-1);
}

void Viewer::on_moveDowButton_Clicked(){
    int id = ui->listWidget->currentItem()->data(0).toInt();
    if(id == frameList.size()-1)
        return;
    emit moveFrame(id, id+1);
    ui->listWidget->setCurrentRow(id+1);
    emit setEditingFrame(id+1);
}

void Viewer::on_colorButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white);
    emit setBrushColor(color);
}

void Viewer::on_addFrameButton_Clicked(){
    addItemToFrameList();
    emit addFrame();
}

void Viewer::on_deleteFrameButton_Clicked(){
    if(frameList.size()==1){
        return;
    }
    int id = ui->listWidget->currentItem()->data(0).toInt();
    delete frameList[id];
    for(int i = id; i<frameList.size()-1; i++){
        frameList[i] = frameList[i+1];
        frameList[i]->setData(0, i);
    }
    frameList.pop_back();
    emit deleteFrame();
}

void Viewer::addItemToFrameList(){
    QListWidgetItem *item = new QListWidgetItem;
    item->setData(0, frameList.size());
    ui->listWidget->addItem(item);
    ui->listWidget->setCurrentItem(item);
    frameList.append(item);
}

void Viewer::onSliderValueChangedSlot(int value){
    ui->FPSLabel->setText( QString("%1 FPS").arg(value));
    emit setPlaybackSpeed(value);
}

void Viewer::on_actionSave_triggered()
{
    QString fileDir = QFileDialog::getSaveFileName(
                this,
                tr("Choose Directory"),
                "C://",
                "Sprite Editor Project (*.ssp);;"
                );
    changed = false;
    emit saveSprite(QString(fileDir));
}


void Viewer::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "C://",
                "Sprite Editor Project (*.ssp);;"

                );

    emit loadJason(filename);
}

void Viewer::updateFrameList(QList<QImage> icons){
    ui->listWidget->clear();
    frameList = QList<QListWidgetItem*>();

    for(int i = 0; i < icons.size();i++){
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(0, frameList.size());
        ui->listWidget->addItem(item);
        ui->listWidget->setCurrentItem(item);
        frameList.append(item);

        image = icons[i];
        QPixmap p = QPixmap::fromImage(image.scaled(QSize(50, 50), Qt::KeepAspectRatio));
        frameList[i]->setIcon(QIcon(p));
    }
}

void Viewer::on_actionNew_triggered()
{
    if(changed){
        QMessageBox msgBox;
        msgBox.setText("The document has not been modified");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret) {
          case QMessageBox::Save:
              on_actionSave_triggered();
              break;
          case QMessageBox::Discard:
              break;
          case QMessageBox::Cancel:
              return;
          default:
              // should never be reached
            return;
        }

    }



    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}


void Viewer::on_actionResize_triggered()
{
    int size = QInputDialog::getInt(this, "Resize", "Canvas Size(smallest size is 2):");
    if(size >=2 ){
        emit resize(size, size);
    }

}


void Viewer::on_playActualSizeButton_clicked()
{
    actualSize = true;
}


void Viewer::on_playButton_clicked()
{
    actualSize = false;
}

