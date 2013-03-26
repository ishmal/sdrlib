#include <QtWidgets>
#include <QtDeclarative>


#include <sdrlib.h>
 
class Sdr
{
private:

    SdrLib *sdr;

public:

    Sdr()
        {
        sdr = sdrCreate();
        }
        
    virtual ~Sdr()
        {
        sdrDelete(sdr);
        }
    
    void setup(QObject *root)
        {
        QWidget *waterfall = root->findChild<QWidget*>("waterfall");
        }

};

#include "simple.moc"

/**
 * This is the "main" of the gui app
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QDeclarativeView view;
    view.setSource(QUrl::fromLocalFile("simple.qml"));
    QObject *root = view.rootObject();
    Sdr sdr;
    sdr.setup(root);
    view.show();

    return app.exec();
}
 
 