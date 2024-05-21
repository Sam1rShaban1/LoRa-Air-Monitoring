// OptionsBar.js
import './optionsBar.css'

import React from 'react';

const OptionsBar = () => {
  return (
    <div className="options-bar">
      <button className="option-button">Connection</button>
      <button className="option-button">Subscribers</button>
      <button className="option-button">Queries</button>
      {/* Add more buttons or other elements as needed */}
    </div>
  );
};

export default OptionsBar;