/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QLabel>
#include <QPainter>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include "colourscheme.hpp"
#include "diagnosticpanel.hpp"
#include "editor.hpp"

AeDiagnosticPanel::AeDiagnosticPanel(AeEditor *editor) :
    QWidget(editor)
{

    QVBoxLayout* l = new QVBoxLayout(this);
    label_ = new QLabel(this);
    //label_->setReadOnly(true);
    label_->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    //label_->setFrameStyle(QFrame::NoFrame);
    label_->setWordWrap(true);
    l->setSpacing(0);
    l->setMargin(4);
    l->addWidget(label_);
    setLayout(l);
}

QSize AeDiagnosticPanel::sizeHint() const {
    return QSize(width(), label_->heightForWidth(width())+8);
}

void AeDiagnosticPanel::paintEvent(QPaintEvent * e) {
    QPainter painter(this);
    painter.fillRect(e->rect(), colourScheme_->panelBackground());
    painter.setPen(Qt::DotLine);
    painter.drawLine(0, 0, width(), 0);
    QWidget::paintEvent(e);
}

void AeDiagnosticPanel::setColourScheme(const ColourScheme* c) {
    colourScheme_ = c;
    QPalette p(label_->palette());
    p.setColor(QPalette::WindowText, colourScheme_->foreground());
    label_->setPalette(p);
}

void AeDiagnosticPanel::show(QString msg) {
    label_->setText(msg.trimmed());
    QWidget::show();
}
