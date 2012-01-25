var deviceInfo = function() {
    document.getElementById("platform").innerHTML = device.platform;
    document.getElementById("version").innerHTML = device.version;
    document.getElementById("uuid").innerHTML = device.uuid;
    document.getElementById("name").innerHTML = device.name;
    document.getElementById("width").innerHTML = screen.width;
    document.getElementById("height").innerHTML = screen.height;
    document.getElementById("colorDepth").innerHTML = screen.colorDepth;
};

var setAudioState = function(state) {
    document.getElementById("audiostate").innerHTML = state;
}

var mediaFile = null;



function playMedia() {
    mediaFile = new Media("http://audio.ibeat.org/content/p1rj1s/p1rj1s_-_rockGuitar.mp3", onSuccess, onError);
    mediaFile.play();
}

function pauseMedia() {
    mediaFile.pause();
}

function stopMedia() {
    mediaFile.stop();
}

function releaseMedia() {
    mediaFile.release();
}

function getDuration() {
    var dur = mediaFile.getDuration();
    if (dur > 0) {
        document.getElementById('audioDuration').innerHTML = (dur) + " sec";
    }
}

function getPosition() {
    // get my_media position
    mediaFile.getCurrentPosition(
        // success callback
        function(position) {
            if (position > -1) {
                setAudioPosition((position) + " sec");
            }
        },
        // error callback
        function(e) {
            console.log("Error getting pos=" + e);
            setAudioPosition("Error: " + e);
        }
    );
}

// onSuccess Callback
//
function onSuccess() {
    console.log("playAudio():Audio Success");
}

// onError Callback
//
function onError(error) {
    alert('code: '    + error.code    + '\n' +
      'message: ' + error.message + '\n');
}

function init() {
    // the next line makes it impossible to see Contacts on the HTC Evo since it
    // doesn't have a scroll button
    // document.addEventListener("touchmove", preventBehavior, false);
    document.addEventListener("deviceready", deviceInfo, true);
}
