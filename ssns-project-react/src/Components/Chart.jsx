import React from 'react';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Filler,
  Legend,
  scales,
  Ticks,
} from 'chart.js';
import { Line } from 'react-chartjs-2';
import { faker } from '@faker-js/faker';
import { color } from 'chart.js/helpers';
import { useEffect, useState } from 'react';
import { db } from "./db";
import { onValue, ref } from "firebase/database";

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Filler,
  Legend
);

export const options = {
  responsive: true,
  maintainAspectRatio: false,
  plugins: {
    legend: {
      position: 'top',
        labels: {
            color: "white",
        
        }
    },
    title: {
      display: true,
      text: 'Weather Chart',
       color:"white"
    },
  },
  scales: {
    y: {
      display: true,
      min: 0,
      max: 30,
      grid: {
        display: false,
      },
      ticks: {
        stepSize: 10,
        color: "white",
      },
    },
    x: {
      display: true,
      grid: {
        display: false,
      },
      ticks: {
        color: "white",
      },
    },
  },


  
};




export function Chart() {
 
  const [hourlyRecords, sethourlyRecords] = useState([]);

    useEffect(() => {
        const hourlyRef = ref(db, 'weather_station/hourly_weather_records');

        onValue(hourlyRef, (snapshot) => {
          const data = snapshot.val();
          const records = data ? Object.keys(data).map(key => ({ id: key, ...data[key] })) : [];
          sethourlyRecords(records);
        });
      }, []);

      const startHour = hourlyRecords.length > 0 ? hourlyRecords[0].hour : null;

const labels = startHour !== null
  ? Array.from({ length: 24 }, (_, i) => `${(startHour + i) % 24}:00`)
  : Array.from({ length: 24 }, (_, i) => `${i}:00`);

      const data = {
        labels,
        datasets: [
          {
            fill: true,
            label: 'Temperature (°C)',
            fontColor: "white",
            data: hourlyRecords.map(record => parseFloat(record.temperature.toFixed(2))),
            borderColor: '#ffc721',
            backgroundColor: '#ffe86d50',
          },
        ],
       
      };
  return <Line options={options} data={data} />;
}
