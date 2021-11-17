#include "previewer.h"
#include <QDebug>

Previewer::Previewer()
{
    playbackSpeed = 24;
    playbackPointer = 0;
}
void Previewer::setSprite(Sprite* address){
    targetSprite = address;
}
void Previewer::setPlaybackSpeed(int speed){
    playbackSpeed = speed;
}
void Previewer::startPlayback(bool play){
    if(play&&!playing){
        playing = play;
        playbackLoop();
    }else{
        playing = play;
    }
}
void Previewer::playbackLoop(){
    if(!playing){
        return;
    }
    emit playback(targetSprite->getPlaybackImage(playbackPointer));
    playbackPointer++;
    if(playbackPointer==targetSprite->getMaxFrame()){
        playbackPointer=0;
    }
    QTimer::singleShot(1000/playbackSpeed, this, SLOT(playbackLoop()) );//fps = 1s / amount of frames
}

void Previewer::updatePreviewer(){
    emit updateEditorWindow(targetSprite->getEditingImage(), targetSprite->getEditingFrame());
}
