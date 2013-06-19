#ifndef DIAGNOSTICPANEL_HPP
#define DIAGNOSTICPANEL_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QWidget>

class QLabel;
class AeEditor;
class ColourScheme;
class AeDiagnosticPanel : public QWidget
{
    Q_OBJECT
public:
    explicit AeDiagnosticPanel(AeEditor *editor);
    void show(QString msg);
    void setColourScheme(const ColourScheme* c);

    QSize sizeHint() const;
protected:
    void paintEvent(QPaintEvent *);
signals:
    
public slots:
    

private:
    QLabel* label_;
    const ColourScheme* colourScheme_;
};

#endif // DIAGNOSTICPANEL_HPP
