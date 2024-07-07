// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getFirestore } from 'firebase/firestore';
import { getAnalytics } from "firebase/analytics";
import { getDatabase } from "firebase/database";
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
const firebaseConfig = {
  apiKey: "AIzaSyBI5uI-DlnczMMdn5Z9tgkvrB94cgXW65Q",
  authDomain: "weatherstation-980f2.firebaseapp.com",
  databaseURL: "https://weatherstation-980f2-default-rtdb.europe-west1.firebasedatabase.app",
  projectId: "weatherstation-980f2",
  storageBucket: "weatherstation-980f2.appspot.com",
  messagingSenderId: "946222914180",
  appId: "1:946222914180:web:a690ca5e0baec9d93380d1",
  measurementId: "G-F55M8FB8CY"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const db = getDatabase(app);
export { db };
// console.log(db)
// const analytics = getAnalytics(app);