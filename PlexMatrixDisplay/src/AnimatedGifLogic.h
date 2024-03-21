#ifdef ANIMATEDGIF_MODE

#include <AnimatedGIF.h>

const char *GIF_ART = "/gifArt.gif";
AnimatedGIF gif;
int x_offset, y_offset;

enum GIFState
{
  STATE_IDLE,
  STATE_OPEN_GIF,
  STATE_PLAY_GIF,
  STATE_CLOSE_GIF,
};


GIFState gifState = STATE_IDLE;

void *GIFOpenFile(const char *fname, int32_t *pSize)
{
  myfile = SPIFFS.open(fname);
  if (myfile)
  {
    *pSize = myfile.size();
    return (void *)&myfile;
  }
  return NULL;
}

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
    f->close();
}

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
  if (iBytesRead <= 0)
    return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
}

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
  // Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
}

void deleteGifArt()
{
  if (SPIFFS.exists(GIF_ART) == true)
  {
    Serial.println("Removing existing image");
    SPIFFS.remove(GIF_ART);
  }
}

#endif