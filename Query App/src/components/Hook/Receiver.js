import React, { useEffect, useState } from 'react';
import { List } from 'antd';

const Receiver = ({ payload }) => {
  const [messages, setMessages] = useState([]);

  useEffect(() => {
    if (payload.topic) {
      const messageObject = JSON.parse(payload.message);
      if ("query" in messageObject.data) {
        setMessages(messages => [...messages, messageObject.data.query]);
      }
    }
    
  }, [payload]);

  const renderListItem = (item) => (
    <List.Item>
      <List.Item.Meta
        title={item.topic}
        description={item}
      />
    </List.Item>
  );

  return (
    <>
      <h2>ANSWER</h2>
      <div style={{ maxHeight: '100%', overflowY: 'scroll' }}>
        <List
          size="small"
          bordered
          dataSource={messages}
          renderItem={renderListItem}
        />
      </div>
    </>
  );  
};

export default Receiver;
