const firebaseConfig = {
    apiKey: "AIzaSyBikgnkAJbVYATTqp7_pI5ljLF2V823oW0",
    authDomain: "motoracc-tracker.firebaseapp.com",
    databaseURL: "https://motoracc-tracker-default-rtdb.firebaseio.com",
    projectId: "motoracc-tracker",
    storageBucket: "motoracc-tracker.appspot.com",
    messagingSenderId: "228509508614",
    appId: "1:228509508614:web:ed5d3955f609100f33cf87"
};
firebase.initializeApp(firebaseConfig);

function f__signout() {
    firebase.auth().signOut().then(() => {
        window.location.href = "index.html";
    }).catch((error) => {
        alert(error);
    });
}