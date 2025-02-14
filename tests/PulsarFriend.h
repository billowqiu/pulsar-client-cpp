/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef PULSAR_FRIEND_HPP_
#define PULSAR_FRIEND_HPP_

#include <string>

#include "lib/ClientConnection.h"
#include "lib/ClientImpl.h"
#include "lib/ConsumerConfigurationImpl.h"
#include "lib/ConsumerImpl.h"
#include "lib/MessageImpl.h"
#include "lib/MultiTopicsConsumerImpl.h"
#include "lib/NamespaceName.h"
#include "lib/PartitionedProducerImpl.h"
#include "lib/ProducerImpl.h"
#include "lib/ReaderImpl.h"
#include "lib/stats/ConsumerStatsImpl.h"
#include "lib/stats/ProducerStatsImpl.h"

using std::string;

namespace pulsar {
class PulsarFriend {
   public:
    static ProducerStatsImplPtr getProducerStatsPtr(Producer producer) {
        ProducerImpl* producerImpl = static_cast<ProducerImpl*>(producer.impl_.get());
        return std::static_pointer_cast<ProducerStatsImpl>(producerImpl->producerStatsBasePtr_);
    }

    template <typename T>
    static unsigned long sum(std::map<T, unsigned long> m) {
        unsigned long sum = 0;
        for (typename std::map<T, unsigned long>::iterator iter = m.begin(); iter != m.end(); iter++) {
            sum += iter->second;
        }
        return sum;
    }

    static ConsumerStatsImplPtr getConsumerStatsPtr(Consumer consumer) {
        ConsumerImpl* consumerImpl = static_cast<ConsumerImpl*>(consumer.impl_.get());
        return std::static_pointer_cast<ConsumerStatsImpl>(consumerImpl->consumerStatsBasePtr_);
    }

    static std::vector<ConsumerStatsImplPtr> getConsumerStatsPtrList(Consumer consumer) {
        if (MultiTopicsConsumerImpl* multiTopicsConsumer =
                dynamic_cast<MultiTopicsConsumerImpl*>(consumer.impl_.get())) {
            std::vector<ConsumerStatsImplPtr> consumerStatsList;
            for (const auto& kv : multiTopicsConsumer->consumers_.toPairVector()) {
                auto consumerStats =
                    std::static_pointer_cast<ConsumerStatsImpl>(kv.second->consumerStatsBasePtr_);
                consumerStatsList.emplace_back(consumerStats);
            }
            return consumerStatsList;

        } else {
            throw std::runtime_error("Consumer must is MultiTopicConsumer.");
        }
    }

    static ProducerImpl& getProducerImpl(Producer producer) {
        ProducerImpl* producerImpl = static_cast<ProducerImpl*>(producer.impl_.get());
        return *producerImpl;
    }

    static int getPartitionProducerSize(Producer producer) {
        PartitionedProducerImpl* partitionedProducerImpl =
            static_cast<PartitionedProducerImpl*>(producer.impl_.get());
        return partitionedProducerImpl->producers_.size();
    }

    static ProducerImpl& getInternalProducerImpl(Producer producer, int index) {
        PartitionedProducerImpl* producerImpl = static_cast<PartitionedProducerImpl*>(producer.impl_.get());
        return *(producerImpl->producers_[index]);
    }

    static void producerFailMessages(Producer producer, Result result) {
        producer.producerFailMessages(result);
    }

    static ConsumerImpl& getConsumerImpl(Consumer consumer) {
        ConsumerImpl* consumerImpl = static_cast<ConsumerImpl*>(consumer.impl_.get());
        return *consumerImpl;
    }

    static std::shared_ptr<ConsumerImpl> getConsumerImplPtr(Consumer consumer) {
        return std::static_pointer_cast<ConsumerImpl>(consumer.impl_);
    }

    static ConsumerImplPtr getConsumer(Reader reader) {
        return std::static_pointer_cast<ConsumerImpl>(reader.impl_->getConsumer());
    }

    static ReaderImplWeakPtr getReaderImplWeakPtr(Reader reader) { return reader.impl_; }

    static decltype(ConsumerImpl::chunkedMessageCache_)& getChunkedMessageCache(Consumer consumer) {
        auto consumerImpl = getConsumerImplPtr(consumer);
        ConsumerImpl::Lock lock(consumerImpl->chunkProcessMutex_);
        return consumerImpl->chunkedMessageCache_;
    }

    static std::shared_ptr<MultiTopicsConsumerImpl> getMultiTopicsConsumerImplPtr(Consumer consumer) {
        return std::static_pointer_cast<MultiTopicsConsumerImpl>(consumer.impl_);
    }

    static std::shared_ptr<ClientImpl> getClientImplPtr(Client client) { return client.impl_; }

    static auto getProducers(const Client& client) -> decltype(ClientImpl::producers_)& {
        return getClientImplPtr(client)->producers_;
    }

    static auto getConsumers(const Client& client) -> decltype(ClientImpl::consumers_)& {
        return getClientImplPtr(client)->consumers_;
    }

    static std::vector<ClientConnectionPtr> getConnections(const Client& client) {
        auto& pool = client.impl_->pool_;
        std::vector<ClientConnectionPtr> connections;
        std::lock_guard<std::recursive_mutex> lock(pool.mutex_);
        for (const auto& kv : pool.pool_) {
            connections.emplace_back(kv.second);
        }
        return connections;
    }

    static std::vector<ProducerImplPtr> getProducers(const ClientConnection& cnx) {
        std::vector<ProducerImplPtr> producers;
        std::lock_guard<std::mutex> lock(cnx.mutex_);
        for (const auto& kv : cnx.producers_) {
            producers.emplace_back(kv.second.lock());
        }
        return producers;
    }

    static std::vector<ConsumerImplPtr> getConsumers(const ClientConnection& cnx) {
        std::vector<ConsumerImplPtr> consumers;
        std::lock_guard<std::mutex> lock(cnx.mutex_);
        for (const auto& kv : cnx.consumers_) {
            consumers.emplace_back(kv.second.lock());
        }
        return consumers;
    }

    static void setNegativeAckEnabled(Consumer consumer, bool enabled) {
        consumer.impl_->setNegativeAcknowledgeEnabledForTesting(enabled);
    }

    static ClientConnectionWeakPtr getClientConnection(HandlerBase& handler) { return handler.connection_; }

    static void setClientConnection(HandlerBase& handler, ClientConnectionWeakPtr conn) {
        handler.connection_ = conn;
    }

    static boost::posix_time::ptime& getFirstBackoffTime(Backoff& backoff) {
        return backoff.firstBackoffTime_;
    }

    static void setServiceUrlIndex(ServiceNameResolver& resolver, size_t index) { resolver.index_ = index; }

    static void setServiceUrlIndex(const Client& client, size_t index) {
        setServiceUrlIndex(client.impl_->serviceNameResolver_, index);
    }

    static proto::MessageMetadata& getMessageMetadata(Message& message) { return message.impl_->metadata; }

    static std::shared_ptr<MessageIdImpl> getMessageIdImpl(MessageId& msgId) { return msgId.impl_; }

    static void setConsumerUnAckMessagesTimeoutMs(const ConsumerConfiguration& consumerConfiguration,
                                                  long unAckedMessagesTimeoutMs) {
        consumerConfiguration.impl_->unAckedMessagesTimeoutMs = unAckedMessagesTimeoutMs;
    }

    static PartitionedProducerImpl& getPartitionedProducerImpl(Producer producer) {
        PartitionedProducerImpl* partitionedProducer =
            static_cast<PartitionedProducerImpl*>(producer.impl_.get());
        return *partitionedProducer;
    }

    static void updatePartitions(PartitionedProducerImpl& partitionedProducer, int newPartitions) {
        LookupDataResultPtr lookupData = std::make_shared<LookupDataResult>();
        lookupData->setPartitions(newPartitions);
        partitionedProducer.handleGetPartitions(ResultOk, lookupData);
    }
};
}  // namespace pulsar

#endif /* PULSAR_FRIEND_HPP_ */
