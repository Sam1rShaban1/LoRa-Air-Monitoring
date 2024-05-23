import React, { createContext, useEffect, useState } from 'react'
import Connection from './Connection'
import Query from './Query'
import Receiver from './Receiver'
import mqtt from 'mqtt'

export const QosOption = createContext([])
// https://github.com/mqttjs/MQTT.js#qos
const qosOption = [
  {
    label: '0',
    value: 0,
  },
  {
    label: '1',
    value: 1,
  },
  {
    label: '2',
    value: 2,
  },
]

const HookMqtt = () => {
  const [client, setClient] = useState(null)
  const [payload, setPayload] = useState({})
  const [connectStatus, setConnectStatus] = useState('Connect')

  const mqttConnect = (host, mqttOption) => {
    setConnectStatus('Connecting')
    /**
     * if protocol is "ws", connectUrl = "ws://broker.emqx.io:8083/mqtt"
     * if protocol is "wss", connectUrl = "wss://broker.emqx.io:8084/mqtt"
     *
     * /mqtt: MQTT-WebSocket uniformly uses /path as the connection path,
     * which should be specified when connecting, and the path used on EMQX is /mqtt.
     *
     * for more details about "mqtt.connect" method & options,
     * please refer to https://github.com/mqttjs/MQTT.js#mqttconnecturl-options
     */
    setClient(mqtt.connect(host, mqttOption))
  }

  useEffect(() => {
    if (client) {
      // https://github.com/mqttjs/MQTT.js#event-connect
      client.on('connect', () => {
        setConnectStatus('Connected')
        console.log('connection successful')
      })

      // https://github.com/mqttjs/MQTT.js#event-error
      client.on('error', (err) => {
        console.error('Connection error: ', err)
        client.end()
      })

      // https://github.com/mqttjs/MQTT.js#event-reconnect
      client.on('reconnect', () => {
        setConnectStatus('Reconnecting')
      })

      // https://github.com/mqttjs/MQTT.js#event-message
      client.on('message', (topic, message) => {
        const payload = { topic, message: message.toString() }
        setPayload(payload)
        // console.log(`received message: ${message} from topic: ${topic}`)
      })
    }
  }, [client])

  // disconnect
  // https://github.com/mqttjs/MQTT.js#mqttclientendforce-options-callback
  const mqttDisconnect = () => {
    if (client) {
      try {
        client.end(false, () => {
          setConnectStatus('Connect')
          console.log('disconnected successfully')
        })
      } catch (error) {
        console.log('disconnect error:', error)
      }
    }
  }

  // publish message
  // https://github.com/mqttjs/MQTT.js#mqttclientpublishtopic-message-options-callback
  const mqttPublish = (context) => {
    if (client) {
      // topic, QoS & payload for publishing message
      const { node, qos, payload, pubTopic } = context
      const jsonPayload = JSON.stringify({
        data: {
          appPortDst: 41,
          appPortSrc: 41,
          addrDst: node,
          query: payload // Assuming 'payload' is the value of the input field
        }
      });
      console.log(jsonPayload)
      client.publish(pubTopic, jsonPayload, { qos }, (error) => {
        if (error) {
          console.log('Publish error: ', error)
        }
      })
    }
  }

  const mqttSub = (subscription) => {
    if (client) {
      // topic & QoS for MQTT subscribing
      const { subTopic, qos } = subscription
      // subscribe topic
      // https://github.com/mqttjs/MQTT.js#mqttclientsubscribetopictopic-arraytopic-object-options-callback
      client.subscribe(subTopic, { qos }, (error) => {
        if (error) {
          console.log('Subscribe to topics error', error)
          return
        }
        console.log(`Subscribe to topics: ${subTopic}`)
      })
    }
  }

  // unsubscribe topic
  // https://github.com/mqttjs/MQTT.js#mqttclientunsubscribetopictopic-array-options-callback
  // const mqttUnSub = (subscription) => {
  //   if (client) {
  //     const { topic, qos } = subscription
  //     client.unsubscribe(topic, { qos }, (error) => {
  //       if (error) {
  //         console.log('Unsubscribe error', error)
  //         return
  //       }
  //       console.log(`unsubscribed topic: ${topic}`)
  //       setIsSub(false)
  //     })
  //   }
  // }

  return (
    <div style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', height: '100vh', width: '100%'}}>
      <div style={{ width: '100%', padding: '20px' }}>
        <Connection
          connect={mqttConnect}
          disconnect={mqttDisconnect}
          connectBtn={connectStatus}
        />
        <div style={{ height: '2px', backgroundColor: 'gray', margin: '20px 0' }} />
        <QosOption.Provider value={qosOption}>
          <Query publish={mqttPublish} sub={mqttSub} />
        </QosOption.Provider>
        <div style={{ height: '2px', backgroundColor: 'gray', margin: '20px 0' }} /> 
        <Receiver payload={payload} />
      </div>
    </div>
  )
}

export default HookMqtt
