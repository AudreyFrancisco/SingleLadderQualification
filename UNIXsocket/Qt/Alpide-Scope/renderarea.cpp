
#include "renderarea.h"

#include <QPainter>

//! [0]
RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    this->resize(1024,512);

}

void RenderArea::fillMatrix(char *buf, long readBytes)
{
    uint32_t *ptr = (uint32_t *)buf;
    while(((char *)ptr) <= ( buf+readBytes-1 )){
        if((*ptr & 0x80000000) != 0) { // this is an event
            for(int x=0; x<1024; x++) for(int y=0;y<512;y++) if(matrix[x][y] >16)  matrix[x][y] -= 16;
        } else {
            matrix[((*ptr & 0x0007FE00) >> 9)][(*ptr & 0x000001FF)] = 254;
        }
        ptr++;
    }
    repaint();
    return;

}


void RenderArea::paintEvent(QPaintEvent * /* event */)
{
    QPen thePen(Qt::red);
    QPainter painter(this);
    thePen.setWidth(1);
    painter.setPen(thePen);
     for(int x=0;x<1024;x++) {
        for(int y=0;y<512;y++) {
            thePen.setColor(QColor(matrix[x][y],0,0,255));
            painter.setPen(thePen);
            painter.drawPoint(x,y);
        }
    }
}
