#include <QtWidgets>
#include <QQuickView>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QQmlContext>
#include <QPainter>
#include <QImage>
#include <QColor>


#include <sdrlib.h>

class Waterfall : public QQuickPaintedItem
{
    Q_OBJECT

public:

    Waterfall()
        {
        image = QImage(width(), height(), QImage::Format_RGB32);
        for (int i=0 ; i<256 ; i++)
            palette[i] = QColor::fromHsv(i, 255, 255, 255);
        }
        
    virtual ~Waterfall()
        {
        }
        
    virtual void paint(QPainter *painter)
        {
        painter->drawImage(0, 0, image);
        }
        
    void resize(QRect rect)
        {
        image = QImage(rect.width(), rect.height(), QImage::Format_RGB32);
        }
        
public slots:

    void update(unsigned int *ps, int size)
        {
        sayHello("got PS!!\n");
        int width = image.width();
        int topLine = image.height() - 1;
        int byteWidth = image.bytesPerLine();
        int byteCount = image.byteCount();
        uchar *buf = (uchar *)image.constBits();
        memmove(buf, buf+byteWidth, byteCount);
        int psidx = 0;
        int acc = -size;
        for (int i = 0; i < 0 ; i++)
            {
            float v = *ps++;
            acc += width;
            if (acc >= 0)
                {
                }
            }
        }
        
    void sayHello(QString msg)
        {
        qDebug() << msg ;
        }
private:

    QImage image;
    QColor palette[256];
};

 
 
static void powerSpectrumCallback(unsigned int *ps, int size, void *ctx)
{
    Waterfall *wf = (Waterfall *)ctx;
    wf->update(ps, size);
}
 
 
 
class Sdr : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double centerFrequency READ getCenterFrequency WRITE setCenterFrequency)
    Q_PROPERTY(float gain READ getGain WRITE setGain)


public:

    Sdr() : waterfall(NULL)
        {
        sdr = sdrCreate();
        waterfall = NULL;
        }
        
    virtual ~Sdr()
        {
        sdrDelete(sdr);
        delete waterfall;
        }
        
    double getCenterFrequency()
        {
        return sdrGetCenterFrequency(sdr);
        }
    
    void setCenterFrequency(double freq)
        {
        sdrSetCenterFrequency(sdr, freq);
        }
    
    float getGain()
        {
        return sdrGetGain(sdr);
        }
    
    void setGain(float gain)
        {
        sdrSetGain(sdr, gain);
        }
        
    Q_INVOKABLE void start()
        {
        sdrStart(sdr);
        }
    
    Q_INVOKABLE void stop()
        {
        sdrStop(sdr);
        }
    
        
    Q_INVOKABLE void setWaterfall(Waterfall *waterfallElem)
        {
        waterfall = waterfallElem;
        waterfall->sayHello("The quick brown fox");
        qDebug() << "ctx init:%p" << waterfall;
        sdrSetPowerSpectrumFunc(sdr, powerSpectrumCallback, (void *)waterfall);
        }
        
    Q_INVOKABLE void sayHello(QString msg)
        {
        qDebug() << msg ;
        }

private:

    SdrLib *sdr;
    Waterfall *waterfall;

};

#include "simple.moc"



/**
 * This is the "main" of the gui app
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qmlRegisterType<Waterfall>("Sdr", 1,0, "Waterfall");
    Sdr sdr;
    QQuickView view;
    view.rootContext()->setContextProperty("sdr", &sdr);
    view.setSource(QUrl("qrc:/simple.qml"));
    QQuickItem *root = view.rootObject();
    Waterfall *waterfall = root->findChild<Waterfall*>("waterfall");
    printf("peer:%p\n", waterfall);
    sdr.setWaterfall(waterfall);
    view.show();
    return app.exec();
}
 
 