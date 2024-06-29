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
      min: 20,
      max: 30,
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

  // scales: {
  //   y: {
  //     min: 20,
  //     max: 30,
  //     grid: {
  //       color: "white"
  //     },
  //     ticks: {
  //       stepSize: 10,
  //       color: "white"
  //     },
  //   },
  //   x: {
  //     grid: {
  //       color: "white"
  //     },
  //     ticks: {
  //       color: "white"
  //     }
  //   }
  // }
  
};

const labels = ['0:00', '3:00', '6:00', '9:00', '12:00', '15:00', '18:00', '21:00'];

export const data = {
  labels,
  datasets: [
    {
      fill: true,
      label: 'Temperature (°C)',
      fontColor: "white",
      data: labels.map(() => faker.datatype.number({ min: 21, max: 28 })),
      borderColor: '#ffc721',
      backgroundColor: '#ffe86d50',
    },
  ],
 
};

export function Chart() {
  return <Line options={options} data={data} />;
}
