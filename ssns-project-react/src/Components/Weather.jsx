import React from 'react';
import { useEffect, useState } from 'react';
// import { collection, getDocs } from 'firebase/firestore'
import { Chart } from './Chart';
import { db } from "./db";
import { onValue, ref } from "firebase/database";
import Time from './Time';


const Weather = () => {
    const days = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
    const [weeklyRecords, setWeeklyRecords] = useState([]);
    const [currentWeather, setCurrentWeather] = useState([]);

    useEffect(() => {
      const weeklyRef = ref(db, 'weather_station/weekly_weather_records');
      const currentRef = ref(db, 'weather_station/current_weather');
      onValue(currentRef, (snapshot) => {
        const data = snapshot.val();
        setCurrentWeather(data);
      });
      const intervalId = setInterval(() => {
        onValue(currentRef, (snapshot) => {
          const data = snapshot.val();
          setCurrentWeather(data);
        });
      }, 10000);
  
      onValue(weeklyRef, (snapshot) => {
        const data = snapshot.val();
        const records = data ? Object.keys(data).map(key => ({ id: key, ...data[key] })) : [];
        setWeeklyRecords(records);
      });
  
      return () => clearInterval(intervalId);
    }, []);

    return (
        <div className="weather-body">
            
        <div className="container weather-container">
            <div className="card">
                <div className="row">
                    <div className="col-12 left">
                        <div className="row top">
                            <div className="col-lg-7"><h2 className='text-center'>Indoor Temperature</h2></div>
                            <div className="col-lg-5"> <h3><b><Time/></b></h3></div>
                            {/* <div className="col-5"><h3>23.06.2024</h3></div> */}
                            {/* <div className="col">Rain map</div> */}
                        </div>
                        <div className="row">
                            <div className="col-lg-7 temp">{currentWeather?.temperature ? `${currentWeather.temperature.toFixed(2)}` : '...'}<span className='error-span'> ± 0.50</span>°C</div>
                            <div className="col-lg-5 time">
                                <p className='d-flex align-items-center justify-content-center'><div className='icon-img-container'><img src='./images/humidity.png'/></div><div className='img-txt text-left'>Humidity</div><div className='icon-value-container'>  {currentWeather?.humidity ? `${currentWeather.humidity}%` : 'Loading Humidity...'}</div></p>
                                <p className='d-flex align-items-center justify-content-center'> <div className='icon-img-container'><img src='./images/air-quality.png'/> </div><div className='img-txt text-left'>Air Quality</div><div className='icon-value-container'> {currentWeather?.aqi ? `${currentWeather.aqi} AQI` : 'Loading AQI...'}</div></p>
                                <p className='d-flex align-items-center justify-content-center'> <div className='icon-img-container'><img src='./images/gauge.png'/> </div><div className='img-txt text-left'>Pressure</div><div className='icon-value-container'> {currentWeather?.pressure ? `${currentWeather.pressure} kPa` : 'Loading Pressure...'}</div></p>
                            </div>
                        </div>
                        <div className="row bottom">
                            <div className="col"><hr /></div>
                          
                            {weeklyRecords.map(record => (
                                    <div className="col" key={record.id}>
                                    <div className="row">{days[record.day]}</div>
                                    <div className="row"><b>{record.temperature.toFixed(2)}&deg;</b></div>
                                </div>
                        
                            ))}
                           
                            <div className="col"><hr /></div>
                        </div>
                        <div className="row chart-row">
                            <div className='col-12'>
                                <Chart />
                            </div>
                        </div>
                    </div>
                    {/* <div className="col-3 right">
                        <div className="row top">For a month</div>
                        <div className="timely">
                            <div className="row">11:00 &nbsp; <b>-4&deg;</b></div>
                            <div className="row">12:00 &nbsp; <b>-4&deg;</b></div>
                            <div className="row">13:00 &nbsp; <b>-5&deg;</b></div>
                            <div className="row">14:00 &nbsp; <b>-7&deg;</b></div>
                            <div className="row">15:00 &nbsp; <b>-4&deg;</b></div>
                            <div className="row">16:00 &nbsp; <b>-4&deg;</b></div>
                        </div>
                    </div> */}
                </div>
            </div>
        </div></div>
    );
};

export default Weather;
