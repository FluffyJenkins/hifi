//
//  ThreadedAssignment.cpp
//  hifi
//
//  Created by Stephen Birarda on 12/3/2013.
//  Copyright (c) 2013 HighFidelity, Inc. All rights reserved.
//

#include <QTimer>

#include "Logging.h"
#include "ThreadedAssignment.h"

ThreadedAssignment::ThreadedAssignment(const unsigned char* dataBuffer, int numBytes) :
    Assignment(dataBuffer, numBytes),
    _isFinished(false)
{
    
}

void ThreadedAssignment::setFinished(bool isFinished) {
    _isFinished = isFinished;

    if (_isFinished) {
        emit finished();
    }
}

void ThreadedAssignment::init(const char* targetName, NODE_TYPE nodeType) {
    // change the logging target name while the assignment is running
    Logging::setTargetName(targetName);
    
    NodeList* nodeList = NodeList::getInstance();
    nodeList->setOwnerType(nodeType);
    
    QTimer* domainServerTimer = new QTimer(this);
    connect(domainServerTimer, SIGNAL(timeout()), this, SLOT(checkInWithDomainServerOrExit()));
    domainServerTimer->start(DOMAIN_SERVER_CHECK_IN_USECS / 1000);
    
    QTimer* pingNodesTimer = new QTimer(this);
    connect(pingNodesTimer, SIGNAL(timeout()), nodeList, SLOT(pingInactiveNodes()));
    pingNodesTimer->start(PING_INACTIVE_NODE_INTERVAL_USECS / 1000);
    
    QTimer* silentNodeTimer = new QTimer(this);
    connect(silentNodeTimer, SIGNAL(timeout()), nodeList, SLOT(removeSilentNodes()));
    silentNodeTimer->start(NODE_SILENCE_THRESHOLD_USECS / 1000);
}

void ThreadedAssignment::checkInWithDomainServerOrExit() {
    if (NodeList::getInstance()->getNumNoReplyDomainCheckIns() == MAX_SILENT_DOMAIN_SERVER_CHECK_INS) {
        setFinished(true);
    } else {
        NodeList::getInstance()->sendDomainServerCheckIn();
    }
}
