/* -*- mode:c++ -*- ********************************************************
 * file:        BaseSensorApplLayer.h
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: application layer: general class for the application layer
 *              subclass to create your own application layer
 **************************************************************************/

#ifndef BASE_SENSOR_APPL_LAYER_H
#define BASE_SENSOR_APPL_LAYER_H

#include <assert.h>

#include "MiXiMDefs.h"
#include "BaseLayer.h"
#include "SimpleAddress.h"

/**
 * @brief Base class for the application layer
 *
 * This is the generic class for all application layer modules. If you
 * want to implement your own application layer you have to subclass your
 * module from this class.
 *
 * @ingroup applLayer
 * @ingroup baseModules
 *
 * @author Daniel Willkomm
 **/
class MIXIM_API BaseSensorApplLayer : public BaseLayer
{
private:
  	/** @brief Copy constructor is not allowed.
  	 */
    BaseSensorApplLayer(const BaseSensorApplLayer&);
  	/** @brief Assignment operator is not allowed.
  	 */
    BaseSensorApplLayer& operator=(const BaseSensorApplLayer&);

public:
	/** @brief The message kinds this layer uses.*/
	enum BaseSensorApplMessageKinds {
		/** Stores the id on which classes extending BaseSensorApplLayer should
		 * continue their own message kinds.*/
		LAST_BASE_APPL_MESSAGE_KIND = 25000,
	};
	/** @brief The control message kinds this layer uses.*/
	enum BaseSensorApplControlKinds {
		/** Stores the id on which classes extending BaseSensorApplLayer should
		 * continue their own control kinds.*/
		LAST_BASE_APPL_CONTROL_KIND = 25500,
	};
protected:
	/**
	 * @brief Length of the ApplPkt header
	 **/
	int headerLength;

public:
	//Module_Class_Members(BaseSensorApplLayer, BaseLayer, 0);
	BaseSensorApplLayer()
		: BaseLayer()
		, headerLength(0)
	{}
	BaseSensorApplLayer(unsigned stacksize)
		: BaseLayer(stacksize)
		, headerLength(0)
	{}

	/** @brief Initialization of the module and some variables*/
	virtual void initialize(int);

protected:
	/**
	 * @name Handle Messages
	 * @brief Functions to redefine by the programmer
	 *
	 * These are the functions provided to add own functionality to your
	 * modules. These functions are called whenever a self message or a
	 * data message from the upper or lower layer arrives respectively.
	 *
	 **/
	/*@{*/

	/**
	 * @brief Handle self messages such as timer...
	 *
	 * Define this function if you want to process timer or other kinds
	 * of self messages
	 **/
	virtual void handleSelfMsg(cMessage* msg) {
		EV << "BaseSensorApplLayer: handleSelfMsg not redefined; delete msg\n";
		delete msg;
	}

	/**
	 * @brief Handle messages from lower layer
	 *
	 * Redefine this function if you want to process messages from lower
	 * layers.
	 *
	 * The basic application layer just silently deletes all messages it
	 * receives.
	 **/
	virtual void handleLowerMsg(cMessage* msg) {
		EV << "BaseSensorApplLayer: handleLowerMsg not redefined; delete msg\n";
		delete msg;
	}

	/**
	 * @brief Handle control messages from lower layer
	 *
	 * The basic application layer just silently deletes all messages it
	 * receives.
	 **/
	virtual void handleLowerControl(cMessage* msg) {
		EV << "BaseSensorApplLayer: handleLowerControl not redefined; delete msg\n";
		delete msg;
	}

	/** @brief Handle messages from upper layer
	 *
	 * This function is pure virtual here, because there is no
	 * reasonable guess what to do with it by default.
	 */
	virtual void handleUpperMsg(cMessage *msg) {
	    EV << "BaseSensorApplLayer: handleUpperMsg not redefined; delete msg\n";
		delete msg;
	}

	/** @brief Handle control messages from upper layer */
	virtual void handleUpperControl(cMessage *msg) {
	    EV << "BaseSensorApplLayer: handleUpperControl not redefined; delete msg\n";
		delete msg;
	}

	/*@}*/

	/** @brief Sends a message delayed to the lower layer*/
	void sendDelayedDown(cMessage *, simtime_t_cref);

	/**
	 * @brief Return my application layer address
	 *
	 * We use the node module index as application address
	 **/
	virtual LAddress::L3Type myApplAddr() const {
		return LAddress::L3Type( getNode()->getIndex() );
	}

};

#endif
