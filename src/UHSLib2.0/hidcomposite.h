/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Circuits At Home, LTD
Web      :  http://www.circuitsathome.com
e-mail   :  support@circuitsathome.com
 */

#if !defined(__HIDCOMPOSITE_H__)
#define __HIDCOMPOSITE_H__

#include "usbhid.h"
//#include "hidescriptorparser.h"

class HIDComposite : public USBHID {

protected:

        struct ReportParser {
                uint8_t rptId;
                HIDReportParser *rptParser;
        } rptParsers[MAX_REPORT_PARSERS];

        // HID class specific descriptor type and length info obtained from HID descriptor
        HID_CLASS_DESCRIPTOR_LEN_AND_TYPE descrInfo[HID_MAX_HID_CLASS_DESCRIPTORS];

        // Returns HID class specific descriptor length by its type and order number
        uint16_t GetHidClassDescrLen(uint8_t type, uint8_t num);

        struct HIDInterface {
                struct {
                        uint8_t bmInterface : 3;
                        uint8_t bmAltSet : 3;
                        uint8_t bmProtocol : 2;
                };
                uint8_t epIndex[maxEpPerInterface + 1]; // We need to make room for the control endpoint as well
        };

        uint8_t bConfNum; // configuration number
        uint8_t bNumIface; // number of interfaces in the configuration
        uint8_t bNumEP; // total number of EP in the configuration
        uint32_t qNextPollTime; // next poll time
        uint8_t pollInterval;
        bool bPollEnable; // poll enable flag

        static const uint16_t constBuffLen = 64; // event buffer length

        void Initialize();
        HIDInterface* FindInterface(uint8_t iface, uint8_t alt, uint8_t proto);

        void ZeroMemory(uint8_t len, uint8_t *buf);


        EpInfo epInfo[totalEndpoints];
        HIDInterface hidInterfaces[maxHidInterfaces];

        // FIXME: bHasReportId is never set (except to false in constructor)
        //        should probably be in EpInfo, /maybe/ in HIDInterface
        //        but setting it isn't that easy (requires parsing report descriptors)
        bool bHasReportId;

        uint16_t PID, VID; // PID and VID of connected device

        // HID implementation
        HIDReportParser* GetReportParser(uint8_t id);

        virtual uint8_t OnInitSuccessful() {
                return 0;
        };

        virtual void ParseHIDData(USBHID *hid __attribute__((unused)), uint8_t ep __attribute__((unused)), bool is_rpt_id __attribute__((unused)), uint8_t len __attribute__((unused)), uint8_t *buf __attribute__((unused))) {
                return;
        };

public:
        HIDComposite(USBHost *p);

        // HID implementation
        bool SetReportParser(uint8_t id, HIDReportParser *prs);

        // USBDeviceConfig implementation
        uint8_t Init(uint8_t parent, uint8_t port, bool lowspeed);
        uint8_t Release();
        uint8_t Poll();

        virtual uint8_t GetAddress() {
                return bAddress;
        };

        virtual bool isReady() {
                return bPollEnable;
        };

        // UsbConfigXtracter implementation
        void EndpointXtract(uint8_t conf, uint8_t iface, uint8_t alt, uint8_t proto, const USB_ENDPOINT_DESCRIPTOR *ep);

        // Send report - do not mix with SetReport()!
        uint8_t SndRpt(uint16_t nbytes, uint8_t *dataptr);

        // Returns true if we should listen on an interface, false if not
        virtual bool SelectInterface(uint8_t iface, uint8_t proto) = 0;
};

#endif // __HIDCOMPOSITE_H__
