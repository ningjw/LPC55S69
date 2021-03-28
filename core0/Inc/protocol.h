#ifndef __PROTOCOL_H
#define __PROTOCOL_H



uint8_t* ParseProtocol(uint8_t *pMsg);
uint32_t PacketUploadSampleData(uint8_t *txBuf, uint32_t sid);
uint32_t PacketSystemInfo(uint8_t *txBuf);
uint32_t PacketVersionInfo(uint8_t *txBuf);

#endif
