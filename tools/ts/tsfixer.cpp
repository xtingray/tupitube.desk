###########################################################################
#   Project TUPI: Magia 2D                                                #
#   Project Contact: info@maefloresta.com                                 #
#   Project Website: http://www.maefloresta.com                           #
#   Project Leader: Gustav Gonzalez <info@maefloresta.com>                #
#                                                                         #
#   Developers:                                                           #
#   2010:                                                                 #
#    Gustavo Gonzalez / xtingray                                          #
#                                                                         #
#   KTooN's versions:                                                     #
#                                                                         #
#   2006:                                                                 #
#    David Cuadrado                                                       #
#    Jorge Cuadrado                                                       #
#   2003:                                                                 #
#    Fernado Roldan                                                       #
#    Simena Dinas                                                         #
#                                                                         #
#   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       #
#   License:                                                              #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program.  If not, see <http://www.gnu.org/licenses/>. #
###########################################################################

#include "tsfixer.h"

TSFixer::TSFixer(const QString &input, const QString &output)
{
    source = input;
    target = output;
}

TSFixer::~TSFixer()
{
}

void TSFixer::start()
{
    qDebug() << "TSFixer::TSFixer() - Loading source file: " << source;
    qDebug() << "TSFixer::TSFixer() - Loading source target: " << target;

    QDomDocument inputDoc;
    QFile input(source);
    if (!input.open(QIODevice::ReadOnly)) {
        qDebug() << "TSFixer::start() - Fatal Error: Can't open source file . " << source;
        return;
    }

    if (!inputDoc.setContent(&input)) {
        qDebug() << "TSFixer::start() - Fatal Error: Can't load source XML file . " << source;
        input.close();
        return;
    }
    input.close();

    QDomDocument outputDoc;
    QFile output(target);
    if (!output.open(QIODevice::ReadOnly)) {
        qDebug() << "TSFixer::start() - Fatal Error: Can't open target file . " << target;
        return;
    }

    if (!outputDoc.setContent(&output)) {
        qDebug() << "TSFixer::start() - Fatal Error: Can't load target XML file . " << target;
        output.close();
        return;
    }
    output.close();

    QDomElement inputRoot = inputDoc.documentElement();
    QDomNode inputNode = inputRoot.firstChild();

    QString source = "";
    QString translation = "";

    qDebug() << "";

    QList<QString> keys; 

    int counter = 1;
    while (!inputNode.isNull()) {
           QDomElement e = inputNode.toElement();
           if (!e.isNull()) {
               if (e.tagName() == "context") {
                   QDomNode n2 = e.firstChild();
                   while (!n2.isNull()) {
                          QDomElement e2 = n2.toElement();
                          if (e2.tagName() == "message") {
                              QDomNode n3 = e2.firstChild(); 
                              while (!n3.isNull()) {
                                     QDomElement e3 = n3.toElement();
                                     if (e3.tagName() == "source") {
                                         source = e3.text();
                                     } else if (e3.tagName() == "translation") {
                                         translation = e3.text();
                                     }
                                     n3 = n3.nextSibling();
                              }

                              // If key hasn't be updated previously
                              if (!keys.contains(source)) {
                                  outputDoc = updateRecord(counter, outputDoc, source, translation); 
                                  counter++;
                                  keys << source;
                              }
                          }
                          n2 = n2.nextSibling();
                   }
               }
           }
           inputNode = inputNode.nextSibling();
    }

    QFile outFile(target);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
         qDebug("TSFixer::start() - Fatal Error: Failed to open file for writing!");
         return;
    }
  
    QTextStream stream(&outFile);
    stream << outputDoc.toString();
    outFile.close();

    qDebug() << "TSFixer::start() - Done!";
}

QDomDocument TSFixer::updateRecord(int counter, QDomDocument outputDoc, const QString &key, const QString &value)
{
    qDebug() << "Record No. " << counter;
    qDebug() << "Updating record - key: " << key << " - value: " << value;

    QDomElement outputRoot = outputDoc.documentElement();
    QDomNode outputNode = outputRoot.firstChild();

    // QString className = "";
    QString source = "";
    QString translation = "";
    QDomNode oldClass;
    
    QDomImplementation imp;
    QDomDocumentType type = imp.createDocumentType("TS", "0", "");
    QDomDocument newDoc(type);
    QDomProcessingInstruction header = newDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    newDoc.appendChild(header); 

    QDomElement root = newDoc.createElement("TS");
    root.setAttribute("version", "2.1");
    root.setAttribute("language", "es_ES");
    newDoc.appendChild(root);  

    while (!outputNode.isNull()) {
           QDomElement e = outputNode.toElement();
           if (!e.isNull()) {
               if (e.tagName() == "context") {
                   QDomNode n2 = e.firstChild();
                   QDomElement context = newDoc.createElement("context");
                   root.appendChild(context); 
                   while (!n2.isNull()) {
                          QDomElement e2 = n2.toElement();
                          if (e2.tagName() == "name") { // Class Name
                              QDomElement classRecord = newDoc.createElement("name"); 
                              QDomText nameDom = newDoc.createTextNode(e2.text());
                              classRecord.appendChild(nameDom);
                              context.appendChild(classRecord);
                          } else if (e2.tagName() == "message") { // Message
                              QDomElement msg = newDoc.createElement("message");
                              context.appendChild(msg);
                              bool update = false;
                              QDomNode n3 = e2.firstChild();
                              while (!n3.isNull()) {
                                  QDomElement e3 = n3.toElement();
                                  if (e3.tagName() == "location") { // Location 
                                      QDomElement locationRecord = newDoc.createElement("location");
                                      locationRecord.setAttribute("line", e3.attribute("line"));
                                      locationRecord.setAttribute("filename", e3.attribute("filename"));
                                      msg.appendChild(locationRecord);
                                  } else if (e3.tagName() == "source") { // Key
                                      source = e3.text();
                                      QDomElement sourceRecord = newDoc.createElement("source");
                                      QDomText sourceDom = newDoc.createTextNode(source);
                                      sourceRecord.appendChild(sourceDom);
                                      msg.appendChild(sourceRecord);
                                      if (source.compare(key) == 0) {
                                          qDebug() << "*** Found key -> " << key; 
                                          update = true;
                                      }
                                  } else if (e3.tagName() == "translation") { // Translation 
                                      QDomElement translationRecord = newDoc.createElement("translation");
                                      QDomText sourceDom = newDoc.createTextNode(e3.text());
                                      if (update) {
                                          qDebug() << "    Replacing value -> " << value;
                                          sourceDom = newDoc.createTextNode(value);
                                      }
                                      translationRecord.appendChild(sourceDom);
                                      msg.appendChild(translationRecord);
                                  }
                                  n3 = n3.nextSibling();
                              }
                          }
                          n2 = n2.nextSibling();
                   }
               }
           }
           outputNode = outputNode.nextSibling();
    }

    return newDoc;
}
