window.addEventListener("load", () => {
  const btn = document.getElementById("btn");
  let Socket = new WebSocket("ws://" + "192.168.43.241" + ":81/");

  Socket.onopen = () => {
    console.log("Connected to server");
  };

  const NUM_BARS = 3;
  const NUM_NOTES = 16;
  const BAR_WIDTH = 20;
  const THRESHOLDS = [200, 100, 45];

  let audioSource;
  let analyser;

  const container = document.getElementById("container");
  container.style.width = `${NUM_BARS * BAR_WIDTH}px`;

  btn.addEventListener("click", () => {
    const audio = new Audio("f1theme.mp3");
    const audioCtx = new AudioContext();
    audio.play();
    audioSource = audioCtx.createMediaElementSource(audio);
    analyser = audioCtx.createAnalyser();
    audioSource.connect(analyser);
    analyser.connect(audioCtx.destination);

    analyser.fftSize = NUM_NOTES * 2;

    const frequencyData = new Uint8Array(analyser.frequencyBinCount);
    const barBinLength = parseInt(NUM_NOTES / NUM_BARS);

    // create divs for each bar
    for (let i = 0; i < NUM_BARS; i++) {
      const bar = document.createElement("div");
      bar.id = `bar${i}`;
      bar.style.width = `${BAR_WIDTH}px`;
      bar.classList.add("bar");
      container.appendChild(bar);
    }

    // set interval
    const interval = setInterval(() => {
      analyser.getByteFrequencyData(frequencyData);
      let data = "";

      for (let i = 0; i < NUM_BARS; i++) {
        const bar = document.getElementById(`bar${i}`);
        let barMean = 0;
        for (let j = 0; j < barBinLength; j++) {
          barMean += frequencyData[i * barBinLength + j];
        }
        barMean /= barBinLength;
        bar.style.height = `${(barMean * 100) / 256}%`;

        if (barMean > THRESHOLDS[i]) {
          data += "1";
        } else {
          data += "0";
        }
      }
      Socket.send(data);
    }, 300);

    audio.addEventListener("ended", () => {
      clearInterval(interval);
      container.innerHTML = "";
    });
  });
});
