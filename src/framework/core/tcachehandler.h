/*
Copyright (c) 2011, Andre Somers
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Rathenau Instituut, Andre Somers nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANDRE SOMERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR #######; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCACHEHANDLER_H
#define TCACHEHANDLER_H

#include "tglobal.h"

#include <QString>
#include <QVector>
#include <QFlags>
#include <QRandomGenerator>

class TCacheHandler
{
    public:
        enum CompressionMode {
            CompressionAuto,
            CompressionAlways,
            CompressionNever
        };

        enum IntegrityProtectionMode {
            ProtectionNone,
            ProtectionChecksum,
            ProtectionHash
        };

        enum Error {
            ErrorNoError,
            ErrorNoKeySet,
            ErrorUnknownVersion,
            ErrorIntegrityFailed
        };

        TCacheHandler();
        explicit TCacheHandler(quint64 key);

        void setParameter(quint64 key);
        bool hasKey() const { return !parts.isEmpty(); }
        void setCompressionMode(CompressionMode mode) { m_compressionMode = mode; }
        CompressionMode compressionMode() const { return m_compressionMode; }
        void setIntegrityProtectionMode(IntegrityProtectionMode mode) { m_protectionMode = mode; }
        IntegrityProtectionMode integrityProtectionMode() const { return m_protectionMode; }
        Error lastError() const { return m_lastError; }
        QString getRecord(const QString& plaintext);
        QString getRecord(QByteArray plaintext);
        QByteArray saveRecordAsByteArray(const QString& plaintext);
        QByteArray saveRecordAsByteArray(QByteArray plaintext);
        QString setRecord(const QString& cyphertext);
        QByteArray restoreFromByteArray(const QString& cyphertext);
        QString setRecord(QByteArray cypher);
        QByteArray restoreFromByteArray(QByteArray cypher);

        enum CacheFlag{ EncodingFlagNone = 0,
                        EncodingFlagCompression = 0x01,
                        EncodingFlagChecksum = 0x02,
                        EncodingFlagHash = 0x04 };

        Q_DECLARE_FLAGS(CacheFlags, CacheFlag);

    private:
        QRandomGenerator generator;

        void splitParameter();
        quint64 id;
        QVector<char> parts;
        CompressionMode m_compressionMode;
        IntegrityProtectionMode m_protectionMode;
        Error m_lastError;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TCacheHandler::CacheFlags)
#endif // TCacheHandler_H
