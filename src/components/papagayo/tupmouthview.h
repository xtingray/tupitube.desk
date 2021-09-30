#ifndef TUPMOUTHVIEW_H
#define TUPMOUTHVIEW_H

#include <QWidget>

#include "tglobal.h"
#include "tuplipsyncdoc.h"

class Q_DECL_EXPORT TupMouthView : public QWidget
{
	Q_OBJECT

    public:
        explicit TupMouthView(QWidget *parent = nullptr);
        ~TupMouthView();

        void setDocument(TupLipsyncDoc *doc);
        void setMouth(int32 id);

    public slots:
        void onMouthChanged(int id);
        void onFrameChanged(int frame);

    protected:
        void paintEvent(QPaintEvent *event);

    private:
        TupLipsyncDoc *document;
        int32 mouthID;
        int32 frame;
        QHash<QString, QImage *> mouths[4];
};

#endif // TUPMOUTHVIEW_H
