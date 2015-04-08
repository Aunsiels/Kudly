static enum wifiReadState wifiReadState;

enum wifiReadState {
    IDLE,
    RECEIVE_HEADER,
    RECEIVE_RESPONSE,
    RECEIVE_LOG
};

enum state {
    WAIT_FEATURE,
    WRITE_FEATURE,
    WAIT_FUNCTION,
    WRITE_FUNCTION,
    END_FEATURE
};

void parseXML(char c) {
    static int parse_feature = 0;
    static int parse_function = 0;
    static enum state state = WAIT_FEATURE;

    switch(state) {
        case WAIT_FEATURE:
            if (c == '<') {
                parse_feature = 0;
                state = WRITE_FEATURE;
            }

        case WRITE_FEATURE:
            if (c == '>'){
                state = WAIT_FUNCTION;
                feature[parse_feature] = '\0';
            }
            feature[parse_feature] = c;
            parse_feature++;

        case WAIT_FUNCTION:
            if (c == '<'){
                state = WRITE_FUNCTION;
                parse_function = 0;
            } 

        case WRITE_FUNCTION:
            if (c == '>'){
                state = END_FEATURE;
                function[parse_function-1] = '\0';
                chSysLock();
                chEvtBroadcastI(&eventWifiSrc);
                chSysUnlock();
            }
            function[parse_function]=c;
            parse_function++;

        case END_FEATURE:
            if (c == '>'){
                state = WAIT_FEATURE;
            }
    }
}


/* Thread that always reads wifi received data */
void wifiMsgParsing(char c) {
    (void)args;
    static int  h;
    static char header[5];
    static int  headerSize;
    static int  errCode;
    static char rcvType;
    static int  dataCpt;
    
    switch(wifiReadState) {
    case IDLE:
	//Message beginning
	if(c == 'R' || c == 'L' || c == 'S') {
	    wifiReadState = RECEIVE_HEADER;
	    rcvType = c;
	    h = 0;
	}
	break;
    case RECEIVE_HEADER:
	
	switch(h) {
	case 0: // Error code
	    errCode = (int)(c - 48);
	    (void)errCode;
	    break;
	case 1: case 2: case 3: case 4: // Receiving header
	    header[h-1] = c;
	    break;
	case 5: // Last header character
	    header[h-1] = c;
	    headerSize = strtol(header, (char **)NULL, 10);
	    dataCpt = 0;
	    break;
	case 7: // After receiving \n\r
	    if(rcvType == 'R') {
		wifiReadState = RECEIVE_RESPONSE;
	    } else {
		wifiReadState = RECEIVE_LOG;
	    }
	    break;
	}
        
	h++;
	break;
    case RECEIVE_RESPONSE:
	writeSerial("%c", c);
	
	dataCpt++;
	if(dataCpt == headerSize) {
	    chSysLock();
	    chEvtBroadcastI(&eventWifiReceptionEnd);
	    chSysUnlock();
	    wifiReadState = IDLE;
	}
	break;
    case RECEIVE_LOG:
	writeSerial("%c", c);
	
	dataCpt++;
	if(dataCpt == headerSize) {
	    writeSerial("LOG !\n\r");
	    chSysLock();
	    chEvtBroadcastI(&eventWifiReceivedLog);
	    chSysUnlock();
	    wifiReadState = IDLE;
	}
	break;
    }
    
    return 0;
}
