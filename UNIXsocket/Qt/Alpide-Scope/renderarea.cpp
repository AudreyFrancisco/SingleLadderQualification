
#include "renderarea.h"

#include <QPainter>

//! [0]
RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{

    this->setGeometry(5,50, 1024, 512);
   // this->resize(1024,512);

}

void RenderArea::fillMatrix(char *buf, long readBytes)
{
    uint32_t *ptr = (uint32_t *)buf;
    while(((char *)ptr) <= ( buf+readBytes-1 )){
        int chipId =  (*ptr >> 19) & 0xF;

        if((*ptr & 0x80000000) == 0) { // this is an event
           int x = ((*ptr & 0x0007FE00) >> 9);
           int y = (*ptr & 0x000001FF);
           if( matrix[chipId][x][y] < 254 * 256)  matrix[chipId][x][y]++ ;
        }

        /*

         if((*ptr & 0x80000000) != 0) { // this is an event
            for(int x=0; x<1024; x++) for(int y=0;y<512;y++) if(matrix[chipId][x][y] >0)  matrix[chipId][x][y] -= 1;
        } else {
            matrix[chipId][((*ptr & 0x0007FE00) >> 9)][(*ptr & 0x000001FF)] = 254 * 256;
        }
*/
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
            thePen.setColor(QColor(matrix[theChipId][x][y] / 256 ,matrix[theChipId][x][y] % 256 , 0 ,255));
            painter.setPen(thePen);
            painter.drawPoint(x,y);
        }
    }
}
