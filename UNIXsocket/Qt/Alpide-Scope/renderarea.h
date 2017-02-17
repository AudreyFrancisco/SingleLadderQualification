#ifndef RENDERAREA_H
#define RENDERAREA_H


#include <QWidget>

class RenderArea : public QWidget
{
    Q_OBJECT

public:
    RenderArea(QWidget *parent);
    void fillMatrix(char *buf, long readBytes);

public slots:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    uint16_t matrix[16][1024][512];
};

#endif // RENDERAREA_H
