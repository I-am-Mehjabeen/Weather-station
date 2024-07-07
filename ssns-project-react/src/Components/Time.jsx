import React, { useState, useEffect } from 'react';
import moment from 'moment-timezone';

const Time = () => {
  const [currentTime, setCurrentTime] = useState(moment().tz('Europe/Berlin').format('LLLL'));

  useEffect(() => {
    const intervalId = setInterval(() => {
      setCurrentTime(moment().tz('Europe/Berlin').format('LLLL'));
    }, 1000); // Update every second

    return () => clearInterval(intervalId);
  }, []);

  return (
    <div>
      <p>{currentTime}</p>
    </div>
  );
};

export default Time;