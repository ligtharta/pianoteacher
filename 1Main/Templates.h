/* https://www.codeproject.com/Articles/257589/An-Idiots-Guide-to-Cplusplus-Templates-Part */

#ifndef Templates_h
#define Templates_h

#include <Arduino.h>



/******************************************************************************************************************************
*
*  TEMPLATE  :  CircularArray
*
*  This CircularArray can store SIZE data parts without the need of 'dynamic memory allocation'.
*  Data elements can be added, iterated and removed (from beginning, or from end). 
*  The order of the elements is always respected.
*
*******************************************************************************************************************************/

template<typename T, byte SIZE> class CircularArray {
  public:
    CircularArray();
    T* add();
    T* getLast();
    T* getFirst();
    void iterateInit(bool forward);
    T* iterate();   
    void removeFirst();
    void removeLast();
    byte count();
    void reset();
#ifdef DEBUG_MODE
    void dump();
#endif
  private:
    T _items[SIZE];                 /* items of circular array */
    byte _idx1, _idx2;
    byte _idx;                      /* index used to iterate over the items */
    bool _forward;                  /* iterate forward or backward? */
};

/* Constructor: */
template<typename T, byte SIZE> CircularArray<T, SIZE>::CircularArray() {
  /* if 1dx1 == idx2, then -> no items available */
  _idx1 = 0;     /* index of item to be processed first */
  _idx2 = 0;     /* index of new item to be added */
  _idx = -1;     /* index used to iterate over the items */
}


/* Add a new item */
template<typename T, byte SIZE> T* CircularArray<T, SIZE>::add() {
  T* item = &_items[_idx2];
  _idx2 = (_idx2 + 1) % SIZE;
  return item;
}


template<typename T, byte SIZE> T* CircularArray<T, SIZE>::getFirst() {
  if (_idx1 == _idx2) return NULL;             /* no items at all */
  T* item = &_items[_idx1];
  return item;    
}

template<typename T, byte SIZE> T* CircularArray<T, SIZE>::getLast() {
  if (_idx1 == _idx2) return NULL;             /* no items at all */
  if (_idx2 == 0) return &_items[SIZE - 1];
  return &_items[_idx2 - 1];
}



template<typename T, byte SIZE> void CircularArray<T, SIZE>::iterateInit(bool forward) {
  _forward = forward;
  _idx = (forward ? _idx1 : _idx2);
}


template<typename T, byte SIZE> T* CircularArray<T, SIZE>::iterate() {
  if (_forward) {
    if (_idx == _idx2) return NULL;
    T* t = &_items[_idx];
    _idx = (_idx + 1) % SIZE;
    return t;
  }
  else {
    if (_idx == _idx1) return NULL;
    if (_idx == 0) _idx = SIZE - 1; else _idx--;
    return &_items[_idx];
  }
}


template<typename T, byte SIZE> void CircularArray<T, SIZE>::removeFirst() {
  if (_idx1 == _idx2) return;
  _idx1 = (_idx1 + 1) % SIZE;                  /* remove 1st item */  
}


template<typename T, byte SIZE> void CircularArray<T, SIZE>::removeLast() {
  if (_idx1 == _idx2) return;
  if (_idx2 == 0) _idx2 = SIZE - 1; else _idx2--; /* remove last item */    
}


template<typename T, byte SIZE> byte CircularArray<T, SIZE>::count() {
  if (_idx1 == _idx2) return 0;             /* no items at all */
  byte cnt = 0;
  byte i = _idx1;
  do {
    i = (i + 1) % SIZE;
    cnt++;
  } while (i != _idx2);
  return cnt;    
}


template<typename T, byte SIZE> void CircularArray<T, SIZE>::reset() {
  /* if 1dx1 == idx2, then -> no items available */
  _idx1 = 0;     /* index of item to be processed first */
  _idx2 = 0;     /* index of new item to be added */
}

#ifdef DEBUG_MODE  /* dump() function only in debug mode. */

template<typename T, byte SIZE> void CircularArray<T, SIZE>::dump() {
  Serial.println("DUMP:");
  for (byte i = _idx1; i != _idx2; i = (i+1) % SIZE) {
    Serial.println(_items[i]);
  }  
  Serial.print("idx1=");
  Serial.print(_idx1);
  Serial.print(", idx2=");
  Serial.print(_idx2);
  Serial.print(", count=");
  Serial.println(count());
}

#endif // DEBUG_MODE




/******************************************************************************************************************************
*
*  TEMPLATE  :  DelayManager
*
*  This DelayManager is used to 'plan' for things to do in the future, without the need of 'dynamic memory allocation'.
*  A data element, together with a 'wake time' can be added to the DelayManager.
*  The DelayManager can be polled (with current time provided), to get data elements at the right time.
*******************************************************************************************************************************/

template<typename T, int SIZE> class DelayManager {
  public:
    /**
      Constructor.
    */
    DelayManager();
    void add(T item, long wakeTime);
    T* checkForRelease(uint32_t now);
    T* peekFirst(uint32_t &actTime);
    int count();
    void removeFirst();
    void reset();
    void addSuspendedMillis(uint32_t dMillis);
#ifdef DEBUG_MODE
    void dump();                            /* for testing only */
#endif // DEBUG_MODE

  private:
    T items[SIZE];                 /* buffer, sorted and works in circular way */
    uint32_t times[SIZE];     /* buffer, sorted and works in circular way */
    void sort();
    int idx1, idx2;
};


/* Constructor: */
template<typename T, int SIZE> DelayManager<T, SIZE>::DelayManager() {
  /* if 1dx1 == idx2, then -> no items available */
  idx1 = 0;     /* index of item to be processed first */
  idx2 = 0;     /* index of new item to be added */
}

/* Add item and sort so that first item is to be released first */
template<typename T, int SIZE> void DelayManager<T, SIZE>::add(T item, long wakeTime) {
  items[idx2] = item;
  times[idx2] = wakeTime;
  idx2 = (idx2 + 1) % SIZE;
  sort();
}

/* Sort items, first item is to be released first */
template<typename T, int SIZE> void DelayManager<T, SIZE>::sort() {
  bool swapped;
  do {
    swapped = false;
    int j = idx1;
    for (int i = idx1; i != idx2; i = (i + 1) % SIZE) {
      if (i == idx1) continue;  /* wait for 2nd item to compare to */
      /* bubble sort: compare items j and i and swap if wakeTime of j is higher. */
      if (times[j] > times[i]) {
        T item = items[j];              /* swap data part */
        items[j] = items[i];
        items[i] = item;
        uint32_t tmp = times[j];   /* swap time part */
        times[j] = times[i];
        times[i] = tmp;
        swapped = true;
      }
      j = i;
    }
  } while (swapped);
}


template<typename T, int SIZE> T* DelayManager<T, SIZE>::checkForRelease(uint32_t now) {
  if (idx1 == idx2) return NULL;             /* no items at all */
  T* item = &items[idx1];
  if (now < times[idx1]) return NULL;        /* not yet time to release first item */
  idx1 = (idx1 + 1) % SIZE;                  /* remove 1st item */
  return item;
}


template<typename T, int SIZE> T* DelayManager<T, SIZE>::peekFirst(uint32_t &actTime) {
  if (idx1 == idx2) return NULL;             /* no items at all */
  T* item = &items[idx1];
  actTime = times[idx1];
  return item;    
}

template<typename T, int SIZE> int DelayManager<T, SIZE>::count() {
  if (idx1 == idx2) return 0;             /* no items at all */
  int cnt = 0;
  int i = idx1;
  do {
    i = (i + 1) % SIZE;
    cnt++;
  } while (i != idx2);
  return cnt;    
}


template<typename T, int SIZE> void DelayManager<T, SIZE>::removeFirst() {
  if (idx1 != idx2)
    idx1 = (idx1 + 1) % SIZE;                  /* remove 1st item */
}

template<typename T, int SIZE> void DelayManager<T, SIZE>::reset() {
  /* if 1dx1 == idx2, then -> no items available */
  idx1 = 0;     /* index of item to be processed first */
  idx2 = 0;     /* index of new item to be added */
}

template<typename T, int SIZE> void DelayManager<T, SIZE>::addSuspendedMillis(uint32_t dMillis) {
  if (idx1 == idx2) return;             /* no items at all */
  int i = idx1;
  do {
    times[i] += dMillis;
    i = (i + 1) % SIZE;
  } while (i != idx2);  
}

#ifdef DEBUG_MODE

template<typename T, int SIZE> void DelayManager<T, SIZE>::dump() {
  Serial.println("DUMP:");
  for (int i = idx1; i != idx2; i = (i+1) % SIZE) {
    Serial.print(times[i]);
    Serial.print(",");
    Serial.println(items[i]);
  }  
  Serial.print("idx1=");
  Serial.print(idx1);
  Serial.print(", idx2=");
  Serial.print(idx2);
  Serial.print(", count=");
  Serial.println(count());
}
#endif // DEBUG_MODE


#endif // Templates_h
