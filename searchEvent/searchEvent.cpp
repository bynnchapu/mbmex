#include <error.h>
#include <assert.h>
#include <iostream>
#include <string.h>
#include <linux/input.h>
#include <pthread.h>
#include "searchEvent.hpp"
#include "../mbmexBase.hpp"
using namespace std;

searchEvent::searchEvent(const char *pInputPath) {
	tracePrint("searchEvent::searchEvent start");
	int retValue = -1;
	memset(deviceName, 0x00, sizeof(deviceName));
	if (nullptr != pInputPath) {
		inputFd = open(pInputPath, O_RDWR);
		if (0 <= inputFd) {
			retValue = ioctl(inputFd,
							 EVIOCGNAME(sizeof(deviceName)),
							 deviceName);
		} else {
			debugPrint("open error");
		}
		if (0 <= retValue) { 
			retValue = ioctl(inputFd, EVIOCGID, &inputId);
		} else {
			debugPrint("ioctl error EVIOCGNAME");
		}
		if (0 <= retValue){ 
			searchEventSts = true;
		} else {
			retValue = close(inputFd);
			debugPrint("ioctl error EVIOCGID");
		}
	}
	if (false == searchEventSts) {
#ifdef MBMEX_DEBUG_ON
		cout << "pInputPath = " << pInputPath << endl;
		cout << "inputFd =" << inputFd << endl;
#endif //MBMEX_DEBUG_ON
	}
	tracePrint("searchEvent::searchEvent end");
}
searchEvent::~searchEvent() {
	ioctl(inputFd, EVIOCGRAB, 0);
	close(inputFd);
}

int searchEvent::eventTask(void) {
    int readResult = -1;
    struct input_event event;
    cout << "searchEvent start" << endl;
    while(true) {
		readResult = read(inputFd, &event, sizeof(event));
        if (-1 == readResult) {
            debugPrint("read error do stop");
            break;
        } else if (readResult != sizeof(event)) {
            printf("size error readResult = %d", readResult);
            break;
		} else if (EV_KEY == event.type) {
			cout << "----------------------" << endl;
        	cout << "deviceName-> " << endl;
			cout << deviceName << endl;
			cout << "deviceVendor->" << endl;
			cout << "0x" << hex << inputId.vendor << endl;
			cout << "deviceType->" << endl;
			cout << "0x" << hex << inputId.product << endl;
        	cout << "inputEvent.type->" << endl;
			cout << "0x" << hex << event.type << endl;
        	cout << "InputEvent.code->" << endl;
			cout << "0x" << hex << event.code << endl;
			cout << "InputEvent.value->" << endl;
			cout << "0x" << hex << event.value << endl;
			cout << "----------------------" << endl;
        	fflush(stdout);
		} else {
			;
		}
    }
    close(inputFd);
	return readResult;
}

int main(void) {
    int retValue = -1;
    int devNum = 0;
    int lpCnt = 0;
    int searchEvtCnt = 0;
    struct dirent **ppNameList;

    devNum = scandir(devInputDir, &ppNameList, checkEventDevice, versionsort);

    if (0 < devNum) {
        searchEvent *pSearchEvt[devNum];
        for (lpCnt = 0; lpCnt < devNum; lpCnt++) {
            char filename[sizeof(devInputDir) + sizeof((ppNameList[lpCnt])->d_name)];
            memset(filename, 0x00, sizeof(filename));
            snprintf(filename,
                     sizeof(filename),
                     "%s/%s",
                     devInputDir,
                     (ppNameList[lpCnt])->d_name);

            printf("filename = %s\n", filename);
            pSearchEvt[searchEvtCnt] = new searchEvent(filename);
            if (nullptr != pSearchEvt[searchEvtCnt]) {
                if (false == pSearchEvt[searchEvtCnt]->searchEventSts) {
                    delete pSearchEvt[searchEvtCnt];
                } else {
                    searchEvtCnt ++;
                }
            }
        }
        pthread_t handle;  // Thread handle
        for (lpCnt = 0; lpCnt < searchEvtCnt; lpCnt++ ) {
            retValue = pthread_create(&handle,
                                      nullptr,
                                      searchEvent::planchThread,
                                      pSearchEvt[lpCnt]);
        }
        pthread_join(handle, nullptr);
    }
    return retValue;
}


