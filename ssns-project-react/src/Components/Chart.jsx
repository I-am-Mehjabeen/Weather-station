import React, { useEffect, useState } from 'react';
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
} from 'chart.js';
import { Line } from 'react-chartjs-2';
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

const errorBarPlugin = {
  id: 'errorBarPlugin',
  afterDatasetsDraw: (chart, args, options) => {
    const { ctx, chartArea: { top, bottom, left, right }, scales: { x, y } } = chart;

    chart.data.datasets.forEach((dataset, datasetIndex) => {
      const errorBars = dataset.errorBars;

      if (errorBars) {
        ctx.save();
        ctx.strokeStyle = options.color || '#ff0000';
        ctx.lineWidth = options.lineWidth || 1.5;

        dataset.data.forEach((value, index) => {
          const xPos = x.getPixelForValue(index);
          const yPos = y.getPixelForValue(value);
          const yErrorTop = y.getPixelForValue(value + errorBars[index].plus);
          const yErrorBottom = y.getPixelForValue(value - errorBars[index].minus);

      
          ctx.beginPath();
          ctx.moveTo(xPos, yErrorTop);
          ctx.lineTo(xPos, yErrorBottom);
          ctx.stroke();

          
          ctx.beginPath();
          ctx.moveTo(xPos - 3, yErrorTop);
          ctx.lineTo(xPos + 3, yErrorTop);
          ctx.stroke();

         
          ctx.beginPath();
          ctx.moveTo(xPos - 3, yErrorBottom);
          ctx.lineTo(xPos + 3, yErrorBottom);
          ctx.stroke();
        });

        ctx.restore();
      }
    });
  },
};

export const options = {
  responsive: true,
  maintainAspectRatio: false,
  plugins: {
    legend: {
      position: 'top',
      labels: {
        color: "white",
      },
    },
    title: {
      display: true,
      text: 'Weather Chart',
      color: "white",
    },
    errorBarPlugin: {
      color: 'white', 
      lineWidth: 1.0,   
    },
  },
  scales: {
    y: {
      display: true,
      min: 10,
      max: 25,
      grid: {
        display: false,
      },
      ticks: {
        stepSize: 5,
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

ChartJS.register(errorBarPlugin);

export function Chart() {
  const [hourlyRecords, setHourlyRecords] = useState([]);

  useEffect(() => {
    const hourlyRef = ref(db, 'weather_station/hourly_weather_records');
    onValue(hourlyRef, (snapshot) => {
      const data = snapshot.val();
      const records = data ? Object.keys(data).map(key => ({ id: key, ...data[key] })) : [];
      setHourlyRecords(records);
    });
  }, []);

  const startHour = hourlyRecords.length > 0 ? hourlyRecords[0].hour : null;
  const labels = startHour !== null
    ? Array.from({ length: 24 }, (_, i) => `${(startHour + i) % 24}:00`)
    : Array.from({ length: 24 }, (_, i) => `${i}:00`);

  const temperatureData = hourlyRecords.map(record => parseFloat(record.temperature.toFixed(2)));

  const data = {
    labels,
    datasets: [
      {
        fill: true,
        label: 'Temperature (°C)',
        fontColor: "white",
        data: temperatureData,
        borderColor: '#ffc721',
        backgroundColor: '#ffe86d50',
        errorBars: temperatureData.map(() => ({ plus: 1.04, minus: 1.04 })),
      },
    ],
  };

  return <Line options={options} data={data} />;
}
