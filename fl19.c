/*
 *	NAME: frame.c
 *	AUTHOR: madachuan
 *	DISCRIPTION: Frame
 *
 *	MODIFIED:
 *	| DATE		| TIME		| DONE
 *	| Feb. 1st 2016	| Mon. 23:58	| Created by madachuan.
 */

#include <math.h>
#include <string.h>

#include <vxWorks.h>
#include <semLib.h>
#include <msgQLib.h>
#include <msgQEvLib.h>
#include <eventLib.h>
#include <taskLib.h>
#include <ioLib.h>
#include <intLib.h>
#include <iv.h>

#include "fl19.h"
#include "fl19protocol.h"
#include "chk.h"
#include "ll.h"

SEM_ID sbtmr;
MSG_Q_ID mqcanr;
MSG_Q_ID mqcant;
MSG_Q_ID mqacst;
MSG_Q_ID mqfcs1;
MSG_Q_ID mqfcs2;
MSG_Q_ID mqfcs3;
MSG_Q_ID mqfcs4;
MSG_Q_ID mqfcs5;
MSG_Q_ID mqmls;
MSG_Q_ID mqets;
int tcanr;
int tcant;
int tfcs;
int tmls;
int tets;
int tgpsr;
int tacsr;
int tacst;
int tdcan;
int tdgps;
int tdacs;
unsigned short counter;
FCSD fcsd;
MLSD mlsd;
ETSD etsd;

/*
 * =====================\
 * FRIST LEVEL BEGIN	|
 */
void frame(void)
{
	/*------\
	| OS	|
	\------*/
	sbtmr = semBCreate(SEM_EMPTY, SEM_Q_FIFO);
	mqcanr = msgQCreate(1, 13, MSG_Q_FIFO);
	mqcant = msgQCreate(4, 9, MSG_Q_FIFO);
	mqacst = msgQCreate(1, sizeof(fcsd.t.acs), MSG_Q_FIFO);
	mqfcs1 = msgQCreate(1, 15, MSG_Q_FIFO);
	mqfcs2 = msgQCreate(1, sizeof(fcsd.r.gps), MSG_Q_FIFO);
	mqfcs3 = msgQCreate(1, sizeof(fcsd.r.acs), MSG_Q_FIFO);
	mqfcs4 = msgQCreate(1, sizeof(mlsd.r), MSG_Q_FIFO);
	mqfcs5 = msgQCreate(1, sizeof(etsd.r), MSG_Q_FIFO);
	mqmls = msgQCreate(1, sizeof(mlsd.t), MSG_Q_FIFO);
	mqets = msgQCreate(3, sizeof(etsd.t), MSG_Q_FIFO);
	/*--------------\
	| Drivers	|
	\--------------*/
	rly();
	can();
	com();
	tmr();
	/*--------------\
	| Middle	|
	\--------------*/
	tcanr = taskSpawn("canr", 100, VX_FP_TASK, 10000, (FUNCPTR)canr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tcant = taskSpawn("cant", 100, VX_FP_TASK, 10000, (FUNCPTR)cant, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tgpsr = taskSpawn("gpsr", 100, VX_FP_TASK, 10000, (FUNCPTR)gpsr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tacsr = taskSpawn("acsr", 100, VX_FP_TASK, 10000, (FUNCPTR)acsr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tacst = taskSpawn("acst", 100, VX_FP_TASK, 10000, (FUNCPTR)acst, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tfcs = taskSpawn("fcs", 100, VX_FP_TASK, 10000, (FUNCPTR)fcs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	/*--------------\
	| Modules	|
	\--------------*/
	tmls = taskSpawn("mls", 100, VX_FP_TASK, 10000, (FUNCPTR)mls, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tets = taskSpawn("ets", 100, VX_FP_TASK, 10000, (FUNCPTR)ets, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	/*------\
	| Dummy	|
	\------*/
	tdcan = taskSpawn("dcan", 100, VX_FP_TASK, 10000, (FUNCPTR)dcan, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tdgps = taskSpawn("dgps", 100, VX_FP_TASK, 10000, (FUNCPTR)dgps, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tdacs = taskSpawn("dacs", 100, VX_FP_TASK, 10000, (FUNCPTR)dacs, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
 *			|
 * FRIST LEVEL END	|
 * =====================/
 */

/*
 * =====================\
 * SECOND LEVEL BEGIN	|
 */
void rly(void)
{
	unsigned i;
	for (i = 0; i < 8; i++)
		closeJDQ(i + 1);
}

void can(void)
{
	HK_CAN_initialize(1, 9, 0x00, 0x80, 0x0A, 0x00, 0xFF, 0x7F, 0x00, 0xFF, 0xAA, 0x88, 0x81, 0x00, 0x88);
	intConnect((VOIDFUNCPTR*)INUM_TO_IVEC(10 + 0x20), (VOIDFUNCPTR)cani, 0);
	sysIntEnablePIC(10);
}

void com(void)
{
	tyCoXRPCIDrv();
	tyCoXRPCIDevCreate("/tjat/0", 0, 256, 256, 32, 32);
	tyCoXRPCIDevCreate("/tjat/3", 3, 256, 256, 32, 32);
	int com1 = open("/tjat/0", O_RDWR, 0);
	int com4 = open("/tjat/3", O_RDWR, 0);
	ioctl(com1, FIOBAUDRATE, 115200);
	ioctl(com4, FIOBAUDRATE, 115200);
}

void tmr(void)
{
	Tinterrupt(4);
	intConnect((VOIDFUNCPTR*)INUM_TO_IVEC(11 + 0x20), (VOIDFUNCPTR)tmri, 0);
	sysIntEnablePIC(11);
	open_Tint();
}

void canr(void)
{
	unsigned char bufr[13];
	unsigned char buft[15];
	FOREVER {
		msgQReceive(mqcanr, bufr, 13, WAIT_FOREVER);
		unsigned short x = counter;
		if (bufr[0] != 0x88)
			continue;
		memcpy(buft, bufr, 13);
		memcpy(buft + 13, &x, 2);
		msgQSend(mqfcs1, buft, 15, NO_WAIT, MSG_PRI_NORMAL);
	}
}

void cant(void)
{
	unsigned char bufr[9];
	static unsigned char buft[7][13];
	unsigned long id[7] = {0x000A6000, 0x080A2000, 0x10FA0F00, 0x180A1000, 0x200A1000, 0x280A1000, 0xA80A0800};
	FOREVER {
		semTake(sbtmr, WAIT_FOREVER);
		while (ERROR != msgQReceive(mqcant, bufr, 9, NO_WAIT)) {
			unsigned char i = bufr[8];
			unsigned char head = 0x88;
			memcpy(buft[i], &head, 1);
			memcpy(buft[i] + 1, &id[i], 4);
			memcpy(buft[i] + 5, bufr, 8);
		}
		static unsigned j;
		switch (counter % 4) {
		case 0:
			HK_CAN_WRITE(1, buft[0]);
			break;
		case 1:
			HK_CAN_WRITE(1, buft[1]);
			break;
		case 2:
			HK_CAN_WRITE(1, buft[2]);
			/*////////////////////////////ctr*/
			/*////////////////////////////relay*/
			break;
		case 3:
			if (j > 3)
				j = 0;
			HK_CAN_WRITE(1, buft[3 + j]);
			j++;
			break;
		default:
			break;
		}
	}
}

void gpsr(void)
{
	int com4 = open("/tjat/3", O_RDWR, 0);
	static char bufr[256];
	static double buft[14];
	FOREVER {
		unsigned len = 0;
		static unsigned sum;
		len = read(com4, bufr + sum, 256);
		if (sum == 0 && strstr(bufr, "$GPHPD,") != bufr)
			continue;
		sum += len;
		if (sum > 256) {
			sum = 0;
			memset(bufr, 0x00, 256);
			continue;
		}
		char *end = NULL;
		if (!(end = strstr(bufr, "\r\n")))
			continue;
		if (*(end - 3) != '*') {
			sum = 0;
			memset(bufr, 0x00, 256);
			continue;
		}
		if (*(end - 2) >= '0' && *(end - 2) <= '9') {
			if (*(end - 2) - 48 != chk(&bufr[1], end - 4 - bufr) >> 4)
				continue;
		} else if (*(end - 2) >= 'A' && *(end - 2) <= 'F') {
			if (*(end - 2) - 55 != chk(&bufr[1], end - 4 - bufr) >> 4)
				continue;
		} else {
			sum = 0;
			memset(bufr, 0x00, 256);
			continue;
		}
		if (*(end - 1) >= '0' && *(end - 1) <= '9') {
			if (*(end - 1) - 48 != (chk(&bufr[1], end - 4 - bufr) & 0x0F))
				continue;
		} else if (*(end - 1) >= 'A' && *(end - 1) <= 'F') {
			if (*(end - 1) - 55 != (chk(&bufr[1], end - 4 - bufr) & 0x0F))
				continue;
		} else {
			sum = 0;
			memset(bufr, 0x00, 256);
			continue;
		}
		char *p = NULL;
		int i = -1;
		int nega = 1;
		int deci = 0;
		double tmp = 0;
		memset(buft, 0x00, sizeof(fcsd.r.gps));
		for (p = bufr + 6; p < end - 3; p++) {
			if (*p == ',' && *(p + 1) >= '0' && *(p + 1) <= '9') {
				i++;
				nega = 1;
				deci = 0;
				continue;
			}
			if (*p == ',' && *(p + 1) == '-' && *(p + 2) >= '0' && *(p + 2) <= '9') {
				i++;
				p++;
				nega = -1;
				deci = 0;
				continue;
			}
			if (*p == '.' && *(p - 1) >= '0' && *(p - 1) <= '9' && *(p + 1) >= '0' && *(p + 1) <= '9') {
				deci = -1;
				continue;
			}
			if (deci == 1) {
				buft[i] += tmp;
				tmp *= 10;
			} else if (deci == -1) {
				buft[i] += double(*p) * pow(10, deci * n);
				deci--;
			}
		}
		int lockkey = intLock();
		counter = 0;
		msgQSend(mqfcs2, &buft[i], sizeof(fcsd.r.gps), NO_WAIT, MSG_PRI_NORMAL);
		intUnlock(lockkey);
		sum = 0;
		memset(bufr, 0x00, 256);
	}
}

void acsr(void)
{
	int com1 = open("/tjat/0", O_RDWR, 0);
	static unsigned char bufr[256];
	FOREVER {
		unsigned len = 0;
		static unsigned sum;
		len = read(com1, bufr + sum, 256);
		if (sum == 0 && bufr[0] != 0xA5)
			continue;
		sum += len;
		if (sum > 256) {
			sum = 0;
			memset(bufr, 0x00, 256);
			continue;
		}
		if (sum < bufr[1] + 2)
			continue;
		if (bufr[bufr[1] - 2] != 0xEE) {
			sum = 0;
			memset(bufr, 0x00, 256);
			continue;
		}
		if (bufr[bufr[1] - 1] != chkxor(&bufr[1], bufr[1] - 3)) {
			sum = 0;
			memset(bufr, 0x00, 256);
			continue;
		}
		unsigned i;
		for (i = 3; i < bufr[1] - 3 - sizeof(fcsd.r.acs); i++) {
			if (bufr[i] != 0xD3)
				continue;
			if (bufr[i + sizeof(fcsd.r.acs) - 1] != chkxor(&bufr[i + 1], sizeof(fcsd.r.acs) - 2))
				continue;
			msgQSend(mqfcs3, &bufr[i], sizeof(fcsd.r.acs), NO_WAIT, MSG_PRI_NORMAL);
			break;
		}
		sum = 0;
		memset(bufr, 0x00, 256);
	}
}

void acst(void)
{
	FOREVER {
		fcsd.t.acs.head = 0xB5;
		fcsd.t.acs.len = sizeof(fcsd.t.acs);
		fcsd.t.acs.ctr++;
		fcsd.t.acs.end = 0xEE;
		fcsd.t.acs.xor = chkxor(&fcsd.t.acs, sizeof(fcsd.t.acs) - 3);
		int com1 = open("/tjat/0", O_RDWR, 0);
		write(com1, &fcsd.t.acs, sizeof(fcsd.t.acs));
		taskDelay(60);
	}
}

void fcs(void)
{
	static unsigned long ev;
	msgQEvStart(mqfcs1, VXEV01, EVENTS_SEND_IF_FREE);
	msgQEvStart(mqfcs2, VXEV02, EVENTS_SEND_IF_FREE);
	msgQEvStart(mqfcs3, VXEV03, EVENTS_SEND_IF_FREE);
	msgQEvStart(mqfcs4, VXEV04, EVENTS_SEND_IF_FREE);
	msgQEvStart(mqfcs5, VXEV05, EVENTS_SEND_IF_FREE);
	FOREVER {
		eventReceive(VXEV01 | VXEV02 | VXEV03 | VXEV04 | VXEV05, EVENTS_WAIT_ANY, WAIT_FOREVER, &ev);
		if (ev & VXEV01) {
			unsigned char bufr[15];
			msgQReceive(mqfcs1, bufr, 15, NO_WAIT);
			unsigned long id[13] = {0x380A9010, 0x400A9020, 0x48019030, 0x500A9040, 0x580A9050, 0x600A9060, 0x680A9070, 0x700A9080, 0x780A9090, 0x800A90A0, 0x880AC0B0, 0x900A80B0, 0x980A80B0};
			unsigned i;
			for (i = 0; i < 13; i++)
				if (*(unsigned long *)(bufr + 1) == id[i])
					break;
			if (i > 12)
				continue;
			memcpy((unsigned char *)&fcsd.r + 10 * i, bufr + 5, 10);
			unsigned j;
			switch (i) {
			case 0:
				etsd.t.ir.find = fcsd.r.ir.find;
				etsd.t.ir.track = fcsd.r.ir.track;
				etsd.t.ir.err = fcsd.r.ir.err;
				if (fcsd.r.ir.err)
					fcsd.t.acs.errir = 1;
				else
					fcsd.t.acs.errir = 0;
				etsd.t.ir.azi = fcsd.r.ir.azi;
				etsd.t.ir.pit = fcsd.r.ir.pit;
				etsd.t.ir.laser = fcsd.r.ir.laser;
				etsd.t.ir.stamp = ((unsigned long)(fcsd.r.gps.time * 1000) + fcsd.r.ir.stamp) % 30000;
				break;
			case 1:
				etsd.t.sv.mov = fcsd.r.sv.mov;
				etsd.t.sv.err = fcsd.r.sv.err;
				if (fcsd.r.sv.err)
					fcsd.t.acs.errsv = 1;
				else
					fcsd.t.acs.errsv = 0;
				etsd.t.sv.azi = fcsd.r.sv.azi;
				etsd.t.sv.pit = fcsd.r.sv.pit;
				etsd.t.sv.stamp = ((unsigned long)(fcsd.r.gps.time * 1000) + fcsd.r.sv.stamp) % 30000;
				break;
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				mlsd.t.m[i - 2].ajc = fcsd.r.m[i - 2].ajc;
				mlsd.t.m[i - 2].tail = fcsd.r.m[i - 2].tail;
				mlsd.t.m[i - 2].mod = fcsd.r.m[i - 2].mod;
				mlsd.t.m[i - 2].exist = fcsd.r.m[i - 2].exist;
				mlsd.t.m[i - 2].cut = fcsd.r.m[i - 2].cut;
				mlsd.t.m[i - 2].pin0 = fcsd.r.m[i - 2].pin0;
				mlsd.t.m[i - 2].pin1 = fcsd.r.m[i - 2].pin1;
				mlsd.t.m[i - 2].ready = fcsd.r.m[i - 2].ready;
				mlsd.t.m[i - 2].safe = fcsd.r.m[i - 2].safe;
				mlsd.t.m[i - 2].regret = fcsd.r.m[i - 2].regret;
				mlsd.t.m[i - 2].battery = fcsd.r.m[i - 2].battery;
				mlsd.t.m[i - 2].feedback = fcsd.r.m[i - 2].feedback;
				mlsd.t.m[i - 2].engine = fcsd.r.m[i - 2].engine;
				*(unsigned char *)&mlsd.t.m[i - 2].err = *(unsigned char *)&fcsd.r.m[i - 2].err;
				if(*(unsigned char *)&fcsd.r.m[i - 2].err) {
					int lockkey = intLock();
					fcsd.t.acs.mls &= ~(0xC000 >> (i - 2) * 2);
					fcsd.t.acs.mls |= (0x0003 << (7 - (i - 2)) * 2);
					intUnlock(lockkey);
				} else if (!fcsd.r.m[i - 2].exist) {
					fcsd.t.acs.mls &= ~(0xC000 >> (i - 2) * 2);
				} else if (!fcsd.r.m[i - 2].ready) {
					int lockkey = intLock();
					fcsd.t.acs.mls &= ~(0xC000 >> (i - 2) * 2);
					fcsd.t.acs.mls |= (0x0001 << (7 - (i - 2)) * 2);
					intUnlock(lockkey);
				} else {
					int lockkey = intLock();
					fcsd.t.acs.mls &= ~(0xC000 >> (i - 2) * 2);
					fcsd.t.acs.mls |= (0x0002 << (7 - (i - 2)) * 2);
					intUnlock(lockkey);
				}
				for (j = 0; j < 8; j++)
					if (*(unsigned char *)&fcsd.r.m[j].err)
						break;
				if (j < 8)
					fcsd.t.acs.errm = 1;
				else
					fcsd.t.acs.errm = 0;
				for (j = 0; j < 8; j++) {
					if (*(unsigned char *)&fcsd.r.m[j].err)
						etsd.t.m.err |= (0x01 << j);
					else
						etsd.t.m.err &= ~(0x01 << j);
					if (fcsd.r.m[j].ready)
						etsd.t.m.ready |= (0x01 << j);
					else
						etsd.t.m.ready &= ~(0x01 << j);
				}
				mlsd.t.m[i - 2].gas = fcsd.r.m[i - 2].gas;
				break;
			case 10:
				etsd.t.dp.sv1 = fcsd.r.dp.sv1;
				etsd.t.dp.sv2 = fcsd.r.dp.sv2;
				etsd.t.dp.get = fcsd.r.dp.get;
				etsd.t.dp.guide = fcsd.r.dp.guide;
				etsd.t.dp.pos = fcsd.r.dp.pos;
				fcsd.t.acs.num = fcsd.r.dp.num + 1;
				mlsd.t.dp.m8 = fcsd.r.dp.m8;
				mlsd.t.dp.m7 = fcsd.r.dp.m7;
				mlsd.t.dp.m6 = fcsd.r.dp.m6;
				mlsd.t.dp.m5 = fcsd.r.dp.m5;
				mlsd.t.dp.m4 = fcsd.r.dp.m4;
				mlsd.t.dp.m3 = fcsd.r.dp.m3;
				mlsd.t.dp.m2 = fcsd.r.dp.m2;
				mlsd.t.dp.m1 = fcsd.r.dp.m1;
				mlsd.t.dp.launch = fcsd.r.dp.launch;
				mlsd.t.dp.safe = fcsd.r.dp.safe;
				mlsd.t.dp.cage = fcsd.r.dp.cage;
				mlsd.t.dp.chk = fcsd.r.dp.chk;
				mlsd.t.dp.rst = fcsd.r.dp.rst;
				mlsd.t.dp.ajc = fcsd.r.dp.ajc;
				mlsd.t.dp.tail = fcsd.r.dp.tail;
				mlsd.t.dp.mod = fcsd.r.dp.mod;
				etsd.t.dp.rkra = fcsd.r.dp.rkra;
				etsd.t.dp.rkrp = fcsd.r.dp.rkrp;
				if (fcsd.r.dp.laser1hz || fcsd.r.dp.laser5hz)
					etsd.t.dp.laser = 1;
				else
					etsd.t.dp.laser = 0;
				if (fcsd.r.dp.laser1hz)
					etsd.t.ir.freq = etsd.t.dp.freq = 0;
				else if (fcsd.r.dp.laser5hz)
					etsd.t.ir.freq = etsd.t.dp.freq = 1;
				etsd.t.dp.ir = fcsd.r.dp.ir;
				break;
			case 11:
				fcsd.t.acs.stp1l = fcsd.t.dp.stp1l = fcsd.r.dp.stp1l;
				fcsd.t.acs.stp1r = fcsd.t.dp.stp1r = fcsd.r.dp.stp1r;
				fcsd.t.acs.fbd1l = fcsd.t.dp.fbd1l = fcsd.r.dp.fbd1l;
				fcsd.t.acs.fbd1r = fcsd.t.dp.fbd1r = fcsd.r.dp.fbd1r;
				fcsd.t.acs.fbd2l = fcsd.t.dp.fbd2l = fcsd.r.dp.fbd2l;
				fcsd.t.acs.fbd2r = fcsd.t.dp.fbd2r = fcsd.r.dp.fbd2r;
				etsd.t.dp.heading = fcsd.r.dp.heading;
				break;
			case 12:
				etsd.t.dp.lat = fcsd.r.dp.lat;
				etsd.t.dp.ns = fcsd.r.dp.ns;
				etsd.t.dp.lon = fcsd.r.dp.lon;
				etsd.t.dp.ew = fcsd.r.dp.ew;
				etsd.t.dp.alt = fcsd.r.dp.alt;
				break;
			default:
				break;
			}
			msgQSend(mqets, &etsd.t, sizeof(etsd.t), NO_WAIT, MSG_PRI_NORMAL);
			msgQSend(mqmls, &mlsd.t, sizeof(mlsd.t), NO_WAIT, MSG_PRI_NORMAL);
			unsigned k;
			if (i > 1 && i < 10)
				k = 2;
			else if (i > 9)
				k = i - 6;
			else
				k = i;
			unsigned char buft[9];
			buft[8] = k;
			memcpy(buft, (unsigned char *)&fcsd.t + 8 * k, 8);
			msgQSend(mqcant, buft, 9, NO_WAIT, MSG_PRI_NORMAL);
		}
		if (ev & VXEV02) {
			msgQReceive(mqfcs2, &fcsd.r.gps, sizeof(fcsd.r.gps), NO_WAIT);
			etsd.t.gps.stamp = (unsigned long)(fcsd.r.gps.time * 1000) % 30000;
			etsd.t.gps.heading = fcsd.t.dp.heading = (short)(fcsd.r.gps.heading / 360 * 65536);
			etsd.t.gps.lat = fcsd.t.dp.lat = fcsd.t.acs.lat = (unsigned long)(fabs(fcsd.r.gps.lat) * 60 * 60 * 10);
			if (fcsd.r.gps.lat > 0)
				etsd.t.gps.ns = fcsd.t.dp.ns = fcsd.t.acs.ns = 0;
			else
				etsd.t.gps.ns = fcsd.t.dp.ns = fcsd.t.acs.ns = 1;
			etsd.t.gps.lon = fcsd.t.dp.lon = fcsd.t.acs.lon = (unsigned long)(fabs(fcsd.r.gps.lon) * 60 * 60 * 10);
			if (fcsd.r.gps.lon > 0)
				etsd.t.gps.ew = fcsd.t.dp.ew = fcsd.t.acs.ew = 0;
			else
				etsd.t.gps.ew = fcsd.t.dp.ew = fcsd.t.acs.ew = 1;
			etsd.t.gps.alt = fcsd.t.dp.alt = fcsd.t.acs.alt = (short)(fcsd.r.gps.alt / 10);
			msgQSend(mqets, &etsd.t, sizeof(etsd.t), NO_WAIT, MSG_PRI_NORMAL);
		}
		if (ev & VXEV03) {
			msgQReceive(mqfcs3, &fcsd.r.acs, sizeof(fcsd.r.acs), NO_WAIT);
			etsd.t.acs.lat = fcsd.r.acs.lat;
			etsd.t.acs.ns = fcsd.r.acs.ns;
			etsd.t.acs.lon = fcsd.r.acs.lon;
			etsd.t.acs.ew = fcsd.r.acs.ew;
			etsd.t.acs.hgt = fcsd.t.dp.hgt = fcsd.r.acs.hgt;
			etsd.t.acs.vx = fcsd.r.acs.vx;
			etsd.t.acs.vy = fcsd.r.acs.vy;
			etsd.t.acs.vz = fcsd.r.acs.vz;
			etsd.t.acs.stamp = fcsd.r.acs.stamp;
			fcsd.t.dp.spd = sqrt(pow(fcsd.r.acs.vx, 2) + pow(fcsd.r.acs.vy, 2) + pow(fcsd.r.acs.vz, 2));
			msgQSend(mqets, &etsd.t, sizeof(etsd.t), NO_WAIT, MSG_PRI_NORMAL);
			msgQSend(mqacst, &fcsd.t.acs, sizeof(fcsd.t.acs), NO_WAIT, MSG_PRI_NORMAL);
		}
		if (ev & VXEV04) {
			msgQReceive(mqfcs4, &etsd.r, sizeof(etsd.r), NO_WAIT);
			fcsd.t.ir.ir = etsd.r.ir.ir;
			fcsd.t.ir.get = etsd.r.ir.get;
			fcsd.t.ir.azi = etsd.r.ir.azi;
			fcsd.t.ir.pit = etsd.r.ir.pit;
			fcsd.t.sv.sv = etsd.r.sv.sv1 && etsd.r.sv.sv2;
			fcsd.t.sv.azi = fcsd.t.sv.azi;
			fcsd.t.sv.pit = fcsd.t.sv.pit;
			mlsd.t.ets.svstp = etsd.r.sv.stp;
			mlsd.t.ets.svfbd = etsd.r.sv.fbd;
			mlsd.t.ets.ahead = etsd.r.sv.ahead;
			fcsd.t.dp.direct = etsd.r.guide.direct;
			fcsd.t.dp.attack = etsd.r.guide.attack;
			fcsd.t.dp.remain = etsd.r.guide.remain;
			fcsd.t.dp.dist = etsd.r.guide.dist;
			fcsd.t.acs.ppi = fcsd.t.dp.ppi = etsd.r.sv.azi + (short)(fcsd.r.gps.heading / 360 * 65536);
		}
		if (ev & VXEV05) {
			msgQReceive(mqfcs5, &mlsd.r, sizeof(mlsd.r), NO_WAIT);
			fcsd.t.m.umask = mlsd.r.umask;
			fcsd.t.m.mod = mlsd.r.m[0].mod;
			fcsd.t.m.tail = mlsd.r.m[0].tail;
			fcsd.t.m.ajc = mlsd.r.m[0].ajc;
			unsigned i;
			for (i = 0; i < 8; i++) {
				unsigned long tmp = 0;
				if (mlsd.r.m[i].chk)
					tmp |= 0x00000008;
				else
					tmp &= ~0x00000008;
				if (mlsd.r.m[i].up)
					tmp |= 0x00000004;
				else
					tmp &= ~0x00000004;
				if (mlsd.r.m[i].cage)
					tmp |= 0x00000002;
				else
					tmp &= ~0x00000002;
				if (mlsd.r.m[i].launch)
					tmp |= 0x00000001;
				else
					tmp &= ~0x00000001;
				int lockkey = intLock();
				fcsd.t.m.cmd &= ~(0xF0000000 >> i * 4);
				fcsd.t.m.cmd |= (tmp << (7 - i) * 4);
				intUnlock(lockkey);
			}
		}
	}
}

void mls(void)
{
	FOREVER {
		msgQReceive(mqmls, &mlsd.t, sizeof(mlsd.t), WAIT_FOREVER);
	}
}

void ets(void)
{
	/* ETS Functional Module	*/
}

void dcan(void)
{
	FOREVER {
		semTake(sbtmr, WAIT_FOREVER);
		unsigned char buft[15] = {0x88};
		unsigned long id[13] = {0x380A9010, 0x400A9020, 0x48019030, 0x500A9040, 0x580A9050, 0x600A9060, 0x680A9070, 0x700A9080, 0x780A9090, 0x800A90A0, 0x880AC0B0, 0x900A80B0, 0x980A80B0};
		static unsigned i;
		static unsigned j;
		switch (counter % 4) {
		case 0:
			memcpy(buft + 1, &id[0], 4);
			buft[5] = 0x28;
			buft[6] = 0x00;
			buft[7] = fcsd.t.ir.azi;
			buft[8] = fcsd.t.ir.azi >> 8;
			buft[9] = fcsd.t.ir.pit;
			buft[10] = fcsd.t.ir.pit >> 8;
			buft[11] = 0x00;
			buft[12] = 0x00;
			buft[13] = counter;
			buft[14] = counter >> 8;
			msgQSend(mqfcs1, buft, 15, NO_WAIT, MSG_PRI_NORMAL);
			break;
		case 1:
			memcpy(buft + 1, &id[1], 4);
			if (fcsd.t.sv.sv)
				buft[5] = 0x10;
			else
				buft[5] = 0x00;
			buft[6] = 0x00;
			buft[7] = fcsd.t.sv.azi;
			buft[8] = fcsd.t.sv.azi >> 8;
			buft[9] = fcsd.t.sv.pit;
			buft[10] = fcsd.t.sv.pit >> 8;
			buft[11] = 0x00;
			buft[12] = 0x00;
			buft[13] = counter;
			buft[14] = counter >> 8;
			msgQSend(mqfcs1, buft, 15, NO_WAIT, MSG_PRI_NORMAL);
			break;
		case 2:
			if (i > 7)
				i = 0;
			memcpy(buft + 1, &id[2 + i], 4);
			i++;
			break;
		case 3:
			if (j > 3)
				j = 0;
			if (j < 3)
				memcpy(buft + 1, &id[10 + j], 4);
			switch (j) {
			case 0:
				buft[5] = 0xEB;
				buft[6] = 0x00;
				buft[7] = 0x00;
				buft[8] = 0x00;
				buft[9] = 0x00;
				buft[10] = 0x10;
				buft[11] = 0x00;
				buft[12] = 0x00;
				break;
			case 1:
				memset(buft + 5, 0x00, 8);
				break;
			case 2:
				memset(buft + 5, 0x00, 8);
				break;
			default:
				break;
			}
			buft[13] = counter;
			buft[14] = counter >> 8;
			j++;
			msgQSend(mqfcs1, buft, 15, NO_WAIT, MSG_PRI_NORMAL);
			break;
		default:
			break;
		}
	}
}

void dgps(void)
{
	double buft[14] = {1800,95133.800,135,-57.00,196.49,45.83072500,122.61647500,240,-0.279,-0.944,-1.186,9.577,4,6};
	FOREVER {
		buft[1]++;
		counter = 0;
		msgQSend(mqfcs2, buft, sizeof(buft), NO_WAIT, MSG_PRI_NORMAL);
		taskDelay(60);
	}
}

void dacs(void)
{
	unsigned char buft[24] = {0xA5, 22, 0, 0xD3, 0xFF, 66, 0, 0, 0, 0, 0, 0, 0x64, 0, 3, 3, 3, 0xD8, 0x0E, 0x00, 0xEE, 0x00, 0x00, 0x1C};
	double angle = 0;
	double pi = 3.141592653589793;
	FOREVER {
		buft[2]++;
		angle += (2 * pi / 600 / 50);
		if (angle >= pi)
			angle -= 2 * pi;
		if (angle <= -pi)
			angle += 2 * pi;
		buft[6] = (unsigned long)((sin(angle) * 10 / 111 + 45.83072500) * 36000) & 0x000000FF;
		buft[7] = ((unsigned long)((sin(angle) * 10 / 111 + 45.83072500) * 36000) & 0x0000FF00) >> 8;
		buft[8] = ((unsigned long)((sin(angle) * 10 / 111 + 45.83072500) * 36000) & 0x00FF0000) >> 16;
		buft[9] = (unsigned long)((cos(angle) * 10 / 111 / cos(45.83072500 / 180 * pi) + 122.61647500) * 36000) & 0x000000FF;
		buft[10] = ((unsigned long)((cos(angle) * 10 / 111 / cos(45.83072500 / 180 * pi) + 122.61647500) * 36000) & 0x0000FF00) >> 8;
		buft[11] = ((unsigned long)((cos(angle) * 10 / 111 / cos(45.83072500 / 180 * pi) + 122.61647500) * 36000) & 0x00FF0000) >> 16;
		buft[17] = (unsigned long)(fcsd.r.gps.time * 1000) % 30000;
		buft[18] = ((unsigned long)(fcsd.r.gps.time * 1000) % 30000) >> 8;
		buft[19] = chkxor(buft + 4, 15);
		buft[21] = chkxor(buft + 1, 19);
		msgQSend(mqfcs3, buft + 3, sizeof(buft) - 7, NO_WAIT, MSG_PRI_NORMAL);
		taskDelay(60);
	}
}
/*			|
 * SECOND LEVEL END	|
 * =====================/
 */

/*
 * =====================\
 * THIRD LEVEL BEGIN	|
 */
void cani(void)
{
	static unsigned char buf[13];
	if (!(INTERRUPT() & 0x01))
		return;
	if (!HK_RD_IR(1)) {
		RESET();
		can();
		return;
	}
	HK_CAN_RECEIVE(1, buf);
	msgQSend(mqcanr, buf, 13, NO_WAIT, MSG_PRI_NORMAL);
}

void tmri(void)
{
	counter++;
	if (counter > 29999)
		counter = 0;
	semFlush(sbtmr);
}
/*			|
 *  THIRD LEVEL END	|
 * =====================/
 */