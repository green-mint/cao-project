const btn = document.getElementById('btn');
const audio = new Audio("audio.mp3");
const audioCtx = new AudioContext();
const NUM_BARS = 3;
const NUM_NOTES = 16;
const BAR_WIDTH = 20;

let audioSource;
let analyser;

const container = document.getElementById('container');
container.style.width = `${NUM_BARS * BAR_WIDTH}px`;

btn.addEventListener('click', () => {
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
    const bar = document.createElement('div');
    bar.id = `bar${i}`;
    bar.style.width = `${BAR_WIDTH}px`;
    bar.classList.add('bar');
    container.appendChild(bar);
  }
  
  // set interval
  const interval = setInterval(() => {
    analyser.getByteFrequencyData(frequencyData);
  
    for (let i = 0; i < NUM_BARS; i++) {
      const bar = document.getElementById(`bar${i}`);
      let barMean = 0;
      for (let j = 0; j < barBinLength; j++) {
        barMean += frequencyData[i * barBinLength + j];
      }
      barMean /= barBinLength;
      bar.style.height = `${barMean * 100 / 256}%`;
    }
  }, 30);

  audio.addEventListener('ended', () => {
    clearInterval(interval);
    container.innerHTML = '';
  })
})  

