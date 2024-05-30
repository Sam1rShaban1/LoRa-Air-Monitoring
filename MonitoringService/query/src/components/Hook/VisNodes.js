import React, { useEffect, useState, useRef } from 'react';
import { Button, Input } from 'antd';

const VisNodes = ({ payload, publish }) => {
  const [uniqueNodes, setUniqueNodes] = useState(new Set());
  const nodeTimestamps = useRef(new Map());
  const [showSecondGrid, setShowSecondGrid] = useState(false);
  const [inputValue, setInputValue] = useState('');
  const [buttonText, setButtonText] = useState('Known GW');

  useEffect(() => {
    if (payload.topic) {
      const messageObject = JSON.parse(payload.message);
      const addrSrcInt = parseInt(messageObject.data.addrSrc, 10);

      // Update the timestamp for the node
      nodeTimestamps.current.set(addrSrcInt, Date.now());

      // Update uniqueNodes using functional update to ensure the latest state
      setUniqueNodes(prevNodes => {
        if (!prevNodes.has(addrSrcInt)) {
          return new Set(prevNodes).add(addrSrcInt);
        }
        return prevNodes;
      });
    }
  }, [payload]);

  useEffect(() => {
    const interval = setInterval(() => {
      const currentTime = Date.now();
      const newNodes = new Set();

      // Check each unique node's last message timestamp
      uniqueNodes.forEach(node => {
        const lastTimestamp = nodeTimestamps.current.get(node);
        if (currentTime - lastTimestamp <= 120000) { // 2 minutes = 120000 milliseconds
          newNodes.add(node); // Node is still active
          console.log(`Node ${node} is active.`);
        } else {
          // Node has been inactive for more than 2 minutes, remove it
          nodeTimestamps.current.delete(node);
          console.log(`Node ${node} is inactive and removed.`);
        }
      });

      setUniqueNodes(newNodes);
    }, 10000); // Check every 10 seconds

    return () => clearInterval(interval);
  }, [uniqueNodes]);

  const toggleGrid = () => {
    setShowSecondGrid(prevState => !prevState);
  };

  const handleInputChange = e => {
    setInputValue(e.target.value);
  };

  const handleButtonClick = () => {
    setButtonText(prevText => prevText === 'Known GW' ? 'Unknown GW' : 'Known GW');
  };

  const handlePublish = (node) => {
    const pubTopic = "from-server/" + node;
    const qos = 2;
    const payload = 6;
    const pubValues = { node, qos, payload, pubTopic };
    publish(pubValues);
  };

  return (
    <>
      <h2 style={headerStyle}>AVAILABLE NODES</h2>
      <Button type="primary" style={{ width: '100%' }} onClick={() => { toggleGrid(); handleButtonClick(); }}>{buttonText}</Button>
      <div style={gridContainerStyle}>
        {showSecondGrid
          ? (
              <div>
                <div style={{ marginBottom: '10px' }}>
                  <Input placeholder="Enter GW" value={inputValue} onChange={handleInputChange} />
                  <Button type="primary" onClick={() => handlePublish(inputValue)}>{"Get nodes"}</Button>
                </div>
                {[...uniqueNodes].map((node, index) => (
                  <div key={index} style={nodeStyle}>
                    <div style={nodeTextStyle}>{node}</div>
                  </div>
                ))}
              </div>
            )
          : (
              <div>
                {[...uniqueNodes].map((node, index) => (
                  <div key={index + uniqueNodes.size} style={nodeStyle}>
                    <div style={nodeTextStyle}>{node}</div>
                  </div>
                ))}
              </div>
            )}
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
