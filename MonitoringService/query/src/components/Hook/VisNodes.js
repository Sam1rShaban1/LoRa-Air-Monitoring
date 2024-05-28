import React, { useEffect, useState } from 'react';

const VisNodes = ({ payload }) => {
  const [messages, setMessages] = useState([]);
  const [uniqueNodes, setUniqueNodes] = useState(new Set());

  useEffect(() => {
    if (payload.topic) {
      const messageObject = JSON.parse(payload.message);
      const addrSrcInt = parseInt(messageObject.data.addrSrc, 10);

      if (!uniqueNodes.has(addrSrcInt)) {
        setUniqueNodes(prevNodes => new Set(prevNodes).add(addrSrcInt));

        const newMessage = {
          text: `${addrSrcInt}`,
        };

        setMessages(messages => [newMessage, ...messages]);
      }
    }
  }, [payload, uniqueNodes]);

  return (
    <>
      <h2 style={headerStyle}>AVAILABLE NODES</h2>
      <div style={gridContainerStyle}>
        {messages.map((message, index) => (
          <div key={index} style={nodeStyle}>
            <div style={nodeTextStyle}>{message.text}</div>
          </div>
        ))}
      </div>
    </>
  );
};

// Styles
const gridContainerStyle = {
  display: 'grid',
  gridTemplateColumns: 'repeat(auto-fit, minmax(100px, 1fr))',
  gap: '20px',
  padding: '20px',
  backgroundColor: 'transparent',
  borderRadius: '8px',
};

const nodeStyle = {
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  backgroundColor: '#ffffff',
  border: '1px solid #d9d9d9',
  borderRadius: '8px',
  height: '100px',
};

const nodeTextStyle = {
  fontWeight: 'bold',
  fontSize: '16px',
  color: '#000',
};

const headerStyle = {
  textAlign: 'left',
  marginBottom: '20px',
};

export default VisNodes;
