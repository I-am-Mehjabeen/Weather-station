import React from 'react';
import { Chart } from './Chart';

const Weather = () => {
    return (
        <div className="weather-body">
        <div className="container weather-container">
            <div className="card">
                <div className="row">
                    <div className="col-12 left">
                        <div className="row top">
                            <div className="col-7"><h2 className='text-center'>Indoor Temperature</h2></div>
                            <div className="col-5"><h3>23.06.2024</h3></div>
                            {/* <div className="col">Rain map</div> */}
                        </div>
                        <div className="row">
                            <div className="col-7 temp">22&deg;C</div>
                            <div className="col-5 time">
                                <p>11:00</p>
                                <h2><b>Saturday</b></h2>
                                <p>45% Humidity</p>
                                <p>22 AQI (Air Quality Index)</p>
                            </div>
                        </div>
                        <div className="row bottom">
                            <div className="col"><hr /></div>
                            <div className="col border">
                                <div className="row">Sat</div>
                                <div className="row"><b>23&deg;</b></div>
                            </div>
                            <div className="col">
                                <div className="row">Sun</div>
                                <div className="row"><b>18&deg;</b></div>
                            </div>
                            <div className="col">
                                <div className="row">Mon</div>
                                <div className="row"><b>16&deg;</b></div>
                            </div>
                            <div className="col">
                                <div className="row">Tue</div>
                                <div className="row"><b>25&deg;</b></div>
                            </div>
                            <div className="col">
                                <div className="row">Wed</div>
                                <div className="row"><b>23&deg;</b></div>
                            </div>
                            <div className="col">
                                <div className="row">Thu</div>
                                <div className="row"><b>21&deg;</b></div>
                            </div>
                            <div className="col">
                                <div className="row">Fri</div>
                                <div className="row"><b>13&deg;</b></div>
                            </div>
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
