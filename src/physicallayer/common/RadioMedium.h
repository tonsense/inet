//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_RADIOMEDIUM_H
#define __INET_RADIOMEDIUM_H

#include <vector>
#include <algorithm>
#include <fstream>
#include "IRadioMedium.h"
#include "IArrival.h"
#include "IInterference.h"
#include "IPropagation.h"
#include "IAttenuation.h"
#include "IBackgroundNoise.h"
#include "MACAddress.h"
#include "TrailFigure.h"

namespace inet {

namespace physicallayer {

/**
 * The default implementation of the radio medium interface.
 */
// TODO: add tests for various optimization configurations
class INET_API RadioMedium : public cSimpleModule, public cListener, public IRadioMedium
{
  protected:
    /**
     * Caches the intermediate computation results related to a reception.
     */
    class ReceptionCacheEntry
    {
      public:
        /**
         * The radio frame that was sent to the receiver or NULL.
         */
        const IRadioFrame *frame;
        const IArrival *arrival;
        const IListening *listening;
        const IReception *reception;
        const IInterference *interference;
        const ISynchronizationDecision *synchronizationDecision;
        const IReceptionDecision *receptionDecision;

      public:
        ReceptionCacheEntry() :
            frame(NULL),
            arrival(NULL),
            listening(NULL),
            reception(NULL),
            interference(NULL),
            synchronizationDecision(NULL),
            receptionDecision(NULL)
        {}
    };

    /**
     * Caches the intermediate computation results related to a transmission.
     */
    class TransmissionCacheEntry
    {
      public:
        /**
         * The last moment when this transmission may have any effect on
         * other transmissions by interfering with them.
         */
        simtime_t interferenceEndTime;
        /**
         * The radio frame that was created by the transmitter is never NULL.
         */
        const IRadioFrame *frame;
        /**
         * The figure representing this transmission.
         */
        cOvalFigure *figure;
        /**
         * The list of intermediate reception computation results.
         */
        std::vector<ReceptionCacheEntry> *receptionCacheEntries;

      public:
        TransmissionCacheEntry() :
            frame(NULL),
            figure(NULL),
            receptionCacheEntries(NULL)
        {}
    };

    enum RangeFilterKind {
        RANGE_FILTER_ANYWHERE,
        RANGE_FILTER_INTERFERENCE_RANGE,
        RANGE_FILTER_COMMUNICATION_RANGE,
    };

  public:
    class INeighborCache
    {
      public:
        virtual void addRadio(const IRadio *radio) = 0;

        virtual void removeRadio(const IRadio *radio) = 0;

        virtual void sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame) = 0;
    };

  protected:
    /** @name Parameters that control the behavior of the radio medium. */
    //@{
    /**
     * The propagation model of transmissions is never NULL.
     */
    const IPropagation *propagation;
    /**
     * The path loss model of transmissions is never NULL.
     */
    const IPathLoss *pathLoss;
    /**
     * The obstacle loss model of transmissions or NULL if unused.
     */
    const IObstacleLoss *obstacleLoss;
    /**
     * The attenuation model of transmissions is never NULL.
     */
    const IAttenuation *attenuation;
    /**
     * The background noise model or NULL if unused.
     */
    const IBackgroundNoise *backgroundNoise;
    /**
     * The maximum speed among the radios is in the range [0, +infinity) or
     * NaN if unspecified.
     */
    mps maxSpeed;
    /**
     * The maximum transmission power among the radio transmitters is in the
     * range [0, +infinity) or NaN if unspecified.
     */
    W maxTransmissionPower;
    /**
     * The minimum interference power among the radio receivers is in the
     * range [0, +infinity) or NaN if unspecified.
     */
    W minInterferencePower;
    /**
     * The minimum reception power among the radio receivers is in the range
     * [0, +infinity) or NaN if unspecified.
     */
    W minReceptionPower;
    /**
     * The maximum gain among the radio antennas is in the range [0, +infinity).
     */
    double maxAntennaGain;
    /**
     * The minimum overlapping in time needed to consider two transmissions
     * interfering.
     */
    // TODO: maybe compute from longest frame duration, maximum mobility speed and signal propagation time
    simtime_t minInterferenceTime;
    /**
     * The maximum transmission duration of a radio signal.
     */
    // TODO: maybe compute from maximum bit length and minimum bitrate
    simtime_t maxTransmissionDuration;
    /**
     * The maximum communication range where a transmission can still be
     * potentially successfully received is in the range [0, +infinity) or
     * NaN if unspecified.
     */
    m maxCommunicationRange;
    /**
     * The maximum interference range where a transmission is still considered
     * to some effect on other transmissions is in the range [0, +infinity)
     * or NaN if unspecified.
     */
    m maxInterferenceRange;
    /**
     * The radio medium doesn't send radio frames to a radio if it's outside
     * the provided range.
     */
    RangeFilterKind rangeFilter;
    /**
     * True means the radio medium doesn't send radio frames to a radio if
     * it's neither in receiver nor in transceiver mode.
     */
    bool radioModeFilter;
    /**
     * True means the radio medium doesn't send radio frames to a radio if
     * it listens on the medium in incompatible mode (e.g. different carrier
     * frequency and bandwidth, different modulation, etc.)
     */
    bool listeningFilter;
    /**
     * True means the radio medium doesn't send radio frames to a radio if
     * it the destination mac address differs.
     */
    bool macAddressFilter;
    /**
     * Records all transmissions and receptions into a separate trace file.
     * The file is at ${resultdir}/${configname}-${runnumber}.tlog
     */
    bool recordCommunicationLog;
    /**
     * Displays ongoing communications on the canvas.
     */
    bool displayCommunication;
    /**
     * Leaves graphical trail of successful communication between radios.
     */
    bool leaveCommunicationTrail;
    //@}

    /** @name Timer */
    //@{
    /**
     * The message used to purge internal state and cache.
     */
    cMessage *removeNonInterferingTransmissionsTimer;
    //@}

    /** @name State */
    //@{
    /**
     * The list of radios that transmit and receive radio signals on the
     * radio medium.
     */
    std::vector<const IRadio *> radios;
    /**
     * The list of ongoing transmissions on the radio medium.
     */
    // TODO: consider using an interval graph for receptions (per receiver radio)
    std::vector<const ITransmission *> transmissions;
    //@}

    /** @name Cache */
    //@{
    /**
     * The smallest radio id of all radios.
     */
    int baseRadioId;
    /**
     * The smallest transmission id of all ongoing transmissions.
     */
    int baseTransmissionId;
    /**
     * Caches neighbors for all radios or NULL if turned off.
     */
    mutable INeighborCache *neighborCache;
    /**
     * Caches pre-computed radio signal information for transmissions and
     * radios. The outer vector is indexed by transmission id (offset with
     * base transmission id) and the inner vector is indexed by radio id.
     * Values that are no longer needed are removed from the beginning only.
     * May contain NULL values for not yet pre-computed information.
     */
    mutable std::vector<TransmissionCacheEntry> cache;
    //@}

    /** @name Logging */
    //@{
    /**
     * The output file where communication log is written to.
     */
    std::ofstream communicationLog;
    //@}

    /** @name Graphics     */
    //@{
    /**
     * The list figures representing ongoing communications.
     */
    cGroupFigure *communicationLayer;
    /**
     * The list of trail figures representing successful communications.
     */
    TrailFigure *communcationTrail;
    //@}

    /** @name Statistics */
    //@{
    /**
     * Total number of transmissions.
     */
    mutable long transmissionCount;
    /**
     * Total number of radio frame sends.
     */
    mutable long sendCount;
    /**
     * Total number of reception computations.
     */
    mutable long receptionComputationCount;
    /**
     * Total number of interference computations.
     */
    mutable long interferenceComputationCount;
    /**
     * Total number of synchronization decision computations.
     */
    mutable long synchronizationDecisionComputationCount;
    /**
     * Total number of reception decision computations.
     */
    mutable long receptionDecisionComputationCount;
    /**
     * Total number of listening decision computations.
     */
    mutable long listeningDecisionComputationCount;
    /**
     * Total number of radio signal reception cache queries.
     */
    mutable long cacheReceptionGetCount;
    /**
     * Total number of radio signal reception cache hits.
     */
    mutable long cacheReceptionHitCount;
    /**
     * Total number of radio signal interference cache queries.
     */
    mutable long cacheInterferenceGetCount;
    /**
     * Total number of radio signal interference cache hits.
     */
    mutable long cacheInterferenceHitCount;
    /**
     * Total number of radio signal synchronization decision cache queries.
     */
    mutable long cacheSynchronizationDecisionGetCount;
    /**
     * Total number of radio signal synchronization decision cache hits.
     */
    mutable long cacheSynchronizationDecisionHitCount;
    /**
     * Total number of radio signal reception decision cache queries.
     */
    mutable long cacheReceptionDecisionGetCount;
    /**
     * Total number of radio signal reception decision cache hits.
     */
    mutable long cacheReceptionDecisionHitCount;
    //@}

  protected:
    /** @name Module */
    //@{
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(cMessage *message);
    //@}

    /** @name Cache */
    //@{
    virtual TransmissionCacheEntry *getTransmissionCacheEntry(const ITransmission *transmission) const;
    virtual ReceptionCacheEntry *getReceptionCacheEntry(const IRadio *receiver, const ITransmission *transmission) const;

    virtual const IArrival *getCachedArrival(const IRadio *receiver, const ITransmission *transmission) const;
    virtual void setCachedArrival(const IRadio *receiver, const ITransmission *transmission, const IArrival *arrival) const;
    virtual void removeCachedArrival(const IRadio *receiver, const ITransmission *transmission) const;

    virtual const IListening *getCachedListening(const IRadio *receiver, const ITransmission *transmission) const;
    virtual void setCachedListening(const IRadio *receiver, const ITransmission *transmission, const IListening *listening) const;
    virtual void removeCachedListening(const IRadio *receiver, const ITransmission *transmission) const;

    virtual const IReception *getCachedReception(const IRadio *receiver, const ITransmission *transmission) const;
    virtual void setCachedReception(const IRadio *receiver, const ITransmission *transmission, const IReception *reception) const;
    virtual void removeCachedReception(const IRadio *receiver, const ITransmission *transmission) const;

    virtual const IInterference *getCachedInterference(const IRadio *receiver, const ITransmission *transmission) const;
    virtual void setCachedInterference(const IRadio *receiver, const ITransmission *transmission, const IInterference *interference) const;
    virtual void removeCachedInterference(const IRadio *receiver, const ITransmission *transmission) const;

    virtual const ISynchronizationDecision *getCachedSynchronizationDecision(const IRadio *receiver, const ITransmission *transmission) const;
    virtual void setCachedSynchronizationDecision(const IRadio *receiver, const ITransmission *transmission, const ISynchronizationDecision *decision) const;
    virtual void removeCachedSynchronizationDecision(const IRadio *receiver, const ITransmission *transmission) const;

    virtual const IReceptionDecision *getCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission) const;
    virtual void setCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission, const IReceptionDecision *decision) const;
    virtual void removeCachedReceptionDecision(const IRadio *receiver, const ITransmission *transmission) const;

    virtual void invalidateCachedDecisions(const ITransmission *transmission);
    virtual void invalidateCachedDecision(const IReceptionDecision *decision);
    //@}

    /** @name Limits */
    //@{
    virtual mps computeMaxSpeed() const;

    virtual W computeMaxTransmissionPower() const;
    virtual W computeMinInterferencePower() const;
    virtual W computeMinReceptionPower() const;
    virtual double computeMaxAntennaGain() const;

    virtual const simtime_t computeMinInterferenceTime() const;
    virtual const simtime_t computeMaxTransmissionDuration() const;

    virtual m computeMaxRange(W maxTransmissionPower, W minReceptionPower) const;
    virtual m computeMaxCommunicationRange() const;
    virtual m computeMaxInterferenceRange() const;

    virtual void updateLimits();
    //@}

    /** @name Transmission */
    //@{
    /**
     * Adds a new transmission to the radio medium.
     */
    virtual void addTransmission(const IRadio *transmitter, const ITransmission *transmission);

    /**
     * Sends a copy of the provided radio frame to all affected receivers on
     * the radio medium.
     */
    virtual void sendToAffectedRadios(IRadio *transmitter, const IRadioFrame *frame);
    //@}

    /** @name Reception */
    //@{
    virtual bool isRadioMacAddress(const IRadio *radio, const MACAddress address) const;

    /**
     * Returns true if the radio can potentially receive the transmission
     * successfully. If this function returns false then the radio medium
     * doesn't send a radio frame to this receiver.
     */
    virtual bool isPotentialReceiver(const IRadio *receiver, const ITransmission *transmission) const;

    virtual bool isInCommunicationRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const;
    virtual bool isInInterferenceRange(const ITransmission *transmission, const Coord startPosition, const Coord endPosition) const;

    virtual bool isInterferingTransmission(const ITransmission *transmission, const IListening *listening) const;
    virtual bool isInterferingTransmission(const ITransmission *transmission, const IReception *reception) const;

    virtual void removeNonInterferingTransmissions();

    virtual const std::vector<const IReception *> *computeInterferingReceptions(const IListening *listening, const std::vector<const ITransmission *> *transmissions) const;
    virtual const std::vector<const IReception *> *computeInterferingReceptions(const IReception *reception, const std::vector<const ITransmission *> *transmissions) const;
    virtual const IReception *computeReception(const IRadio *receiver, const ITransmission *transmission) const;
    virtual const IInterference *computeInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const;
    virtual const ISynchronizationDecision *computeSynchronizationDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const;
    virtual const IReceptionDecision *computeReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, const std::vector<const ITransmission *> *transmissions) const;
    virtual const IListeningDecision *computeListeningDecision(const IRadio *receiver, const IListening *listening, const std::vector<const ITransmission *> *transmissions) const;

    virtual const IReception *getReception(const IRadio *receiver, const ITransmission *transmission) const;
    virtual const IInterference *getInterference(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const;
    virtual const ISynchronizationDecision *getSynchronizationDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const;
    virtual const IReceptionDecision *getReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const;
    //@}

    /** @name Graphics */
    //@{
    virtual void updateCanvas();
    //@}

  public:
    RadioMedium();
    virtual ~RadioMedium();

    virtual void printToStream(std::ostream& stream) const { stream << "Radio medium"; }

    virtual W getMinInterferencePower() const { return minInterferencePower; }
    virtual W getMinReceptionPower() const { return minReceptionPower; }
    virtual double getMaxAntennaGain() const { return maxAntennaGain; }

    virtual const Material *getMaterial() const { return &Material::air; }
    virtual const IPropagation *getPropagation() const { return propagation; }
    virtual const IPathLoss *getPathLoss() const { return pathLoss; }
    virtual const IObstacleLoss *getObstacleLoss() const { return obstacleLoss; }
    virtual const IAttenuation *getAttenuation() const { return attenuation; }
    virtual const IBackgroundNoise *getBackgroundNoise() const { return backgroundNoise; }

    virtual void addRadio(const IRadio *radio);
    virtual void removeRadio(const IRadio *radio);

    virtual void sendToRadio(IRadio *trasmitter, const IRadio *receiver, const IRadioFrame *frame);

    virtual IRadioFrame *transmitPacket(const IRadio *transmitter, cPacket *macFrame);
    virtual const ISynchronizationDecision *synchronizePacket(const IRadio *receiver, IRadioFrame *radioFrame);
    virtual cPacket *receivePacket(const IRadio *receiver, IRadioFrame *radioFrame);

    virtual const IListeningDecision *listenOnMedium(const IRadio *receiver, const IListening *listening) const;

    virtual bool isSynchronizationAttempted(const IRadio *receiver, const ITransmission *transmission) const;
    virtual bool isReceptionAttempted(const IRadio *receiver, const ITransmission *transmission) const;

    virtual const IArrival *getArrival(const IRadio *receiver, const ITransmission *transmission) const;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, long value);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RADIOMEDIUM_H

