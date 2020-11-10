/*
  WiFiClient.h - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified by Ivan Grokhotkov, December 2014 - esp8266 support
*/

#ifndef wificlient_h
#define wificlient_h
#include <memory>
#include "Arduino.h"
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"
#include "include/slist.h"

#define WIFICLIENT_MAX_PACKET_SIZE 1460

class ClientContext;
class WiFiServer;

class WiFiClient : public Client, public SList<WiFiClient> {
protected:
  WiFiClient(ClientContext* client);

public:
  WiFiClient();
  virtual ~WiFiClient();
  WiFiClient(const WiFiClient&);
  WiFiClient& operator=(const WiFiClient&);

  uint8_t status();
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t b) { return write(b, true); }
  size_t write(uint8_t, bool last);
  virtual size_t write(const uint8_t *buf, size_t size) { return write(buf, size, true); }
  size_t write(const uint8_t *buf, size_t size, bool last);
  size_t write_P(PGM_P buf, size_t size, bool last);
  size_t write(Stream& stream);

  // This one is deprecated, use write(Stream& instead)
  //size_t write(Stream& stream, size_t unitSize) __attribute__ ((deprecated));
  
   template <typename T>
  size_t write(T& source, size_t unitSize);

  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual size_t peekBytes(uint8_t *buffer, size_t length);
  size_t peekBytes(char *buffer, size_t length) {
    return peekBytes((uint8_t *) buffer, length);
  }
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

  IPAddress remoteIP();
  uint16_t  remotePort();
  IPAddress localIP();
  uint16_t  localPort();
  bool getNoDelay();
  void setNoDelay(bool nodelay);
  static void setLocalPortStart(uint16_t port) { _localPort = port; }

  template<typename T> size_t write(T &src){
    uint8_t obuf[WIFICLIENT_MAX_PACKET_SIZE];
    size_t doneLen = 0;
    size_t sentLen;
    int i;

    while (src.available() > WIFICLIENT_MAX_PACKET_SIZE){
      src.read(obuf, WIFICLIENT_MAX_PACKET_SIZE);
#if 1	//dc42
      sentLen = write(obuf, WIFICLIENT_MAX_PACKET_SIZE, false);
#else
      sentLen = write(obuf, WIFICLIENT_MAX_PACKET_SIZE);
#endif
      doneLen = doneLen + sentLen;
      if(sentLen != WIFICLIENT_MAX_PACKET_SIZE){
        return doneLen;
      }
    }

    uint16_t leftLen = src.available();
    src.read(obuf, leftLen);
#if 0	//dc42
    sentLen = write(obuf, leftLen, true);
#else
    sentLen = write(obuf, leftLen);
#endif
    doneLen = doneLen + sentLen;
    return doneLen;
  }
  
  size_t availableForWrite();

  friend class WiFiServer;

  using Print::write;

  static void stopAll();
  static void stopAllExcept(WiFiClient * c);

protected:

  static int8_t _s_connected(void* arg, void* tpcb, int8_t err);
  static void _s_err(void* arg, int8_t err);

  int8_t _connected(void* tpcb, int8_t err);
  void _err(int8_t err);

  ClientContext* _client;
  static uint16_t _localPort;
};


template <typename T>
inline size_t WiFiClient::write(T& source, size_t unitSize) {

  std::unique_ptr<uint8_t[]> buffer(new uint8_t[unitSize]);
  size_t size_sent = 0;
  while(true) {
    size_t left = source.available();
    if (!left)
      break;
    size_t will_send = (left < unitSize) ? left : unitSize;
    source.read(buffer.get(), will_send);  
#if 1	//dc42
    size_t cb = write(buffer.get(), will_send, will_send == left);
#else
    size_t cb = write(buffer.get(), will_send);
#endif
    size_sent += cb;
    if (cb != will_send) {
      break;
    }
  }
  return size_sent;
}

#endif
