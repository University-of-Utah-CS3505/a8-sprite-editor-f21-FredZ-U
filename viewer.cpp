#include "viewer.h"
#include "ui_viewer.h"

Viewer::Viewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Viewer)
{
    ui->setupUi(this);
    drawingPivot = QPoint(40,40);
    pixelSize = 20;
    pixelOffset = 1;
}

Viewer::~Viewer()
{
    delete ui;
}

void Viewer::playback(const QImage &frameImage){

}
void Viewer::updateEditor(const QImage &frameImage){

    image = frameImage;
    update();

}
void Viewer::saveCallback(bool success){

}
void Viewer::loadCallback(bool success){

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
            qDebug()<<"usetool";
        }

    }else{
        drawingPivot += event->pos() - movePivot;
        movePivot = event->pos();
        repaint();
    }


}


void Viewer::mouseReleaseEvent(QMouseEvent * event){
    if(event->button() == Qt::LeftButton){//if mouse left button clicked

        emit useToolOn(event -> pos());//get mouse position as end point

    }


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
