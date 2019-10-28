const functions = require('firebase-functions');
const secureCompare = require('secure-compare');
const firebase = require('firebase');

//configuring settings for app initialization
var config = {
    apiKey: "AIzaSyCzYLGj7ZElG-fuUWrkJDxguphkwvdvxGY",
    authDomain: "biddlr.firebaseapp.com",
    databaseURL: "https://biddlr.firebaseio.com",
    projectId: "biddlr",
    storageBucket: "biddlr.appspot.com",
    messagingSenderId: "80513958552"
};
firebase.initializeApp(config);

const status_expired = "EXPIRED";
const status_in_progress = "IN_PROGRESS";

//handle to the firebase db for reading and writing
const database = firebase.database();

var ref_active_jobs = database.ref("active_job");
var ref_expired_jobs = database.ref("expired_job");
var ref_in_progress_jobs = database.ref("in_progress_job");

/*
 * When requested this Function will update every job that has timed out in the last minute.
 * The request needs to be authorized by passing a 'key' query parameter in the URL. This key must
 * match a key set as an environment variable using `firebase functions:config:set cron.key="YOUR_KEY"`.
 */
exports.changeJobStatuses = functions.https.onRequest((request, response) => {

    //grab the key provided in the request
    const key = request.query.key;

    // Exit if the key doesn't match function key specified
    if (!secureCompare(key, functions.config().cron.key)) {
        console.log('The key provided in the request does not match the key set in the environment. Check that', key,
            'matches the cron.key attribute in `firebase env:get`');
        response.status(403).send('Security key does not match. Make sure your "key" URL query parameter matches the ' +
            'cron.key environment variable.');
        return null;
    }
    
    //makes call to update jobs function
    ref_active_jobs.orderByChild("expirationDate/milliseconds").endAt(Date.now()).on('child_added', updateTimedOutJobs);

    //confirms that process has been completed to firebase logger
    response.send('Updates Completed');

    return null;
});

//Updates all jobs in the active jobs table whose bidding period has ended
function updateTimedOutJobs(snaphot) {
    var bids = snaphot.val().bids

    //if bids have been placed the job can proceed to in progress
    if (bids !== null && bids !== undefined) {
        moveJobToInProgress(ref_in_progress_jobs, snaphot);
    }
    //no one bid on the job so it must expire
    else {
        moveJobToExpired(ref_expired_jobs, snaphot);
    }
}

//Moves a job to the expired jobs table and deletes it from the active_job table
function moveJobToExpired(reference, jobSnaphot) {
    reference.push({
        jobID: jobSnaphot.val().jobID,
        status: status_expired,
        title: jobSnaphot.val().title,
        description: jobSnaphot.val().description,
        posterID: jobSnaphot.val().posterID,
        location: jobSnaphot.val().location,
        coordinates: jobSnaphot.val().coordinates,
        expirationDate: jobSnaphot.val().expirationDate,
        startingPrice: jobSnaphot.val().startingPrice,
    });

    console.log('Moved job: ' + jobSnaphot.val().jobID + ' to expired table');
    
    //getting a handle to the job instance in the active jobs table and removing it
    var job = database.ref("active_job/" + jobSnaphot.val().jobID);
    job.remove();
}

//Moves a job to the in progress table and deletes it from the active_job table
function moveJobToInProgress(reference, jobSnaphot) {
    reference.push({
        jobID: jobSnaphot.val().jobID,
        status: status_in_progress,
        title: jobSnaphot.val().title,
        description: jobSnaphot.val().description,
        posterID: jobSnaphot.val().posterID,
        location: jobSnaphot.val().location,
        coordinates: jobSnaphot.val().coordinates,
        expirationDate: jobSnaphot.val().expirationDate,
        startingPrice: jobSnaphot.val().startingPrice,
        currentBid: jobSnaphot.val().currentBid,
        bids: jobSnaphot.val().bids
    });

    console.log('Moved job: ' + jobSnaphot.val().jobID + ' to in progress table');
    
    //getting a handle to the job instance in the active jobs table and removing it
    var job = database.ref("active_job/" + jobSnaphot.val().jobID);
    job.remove();
}