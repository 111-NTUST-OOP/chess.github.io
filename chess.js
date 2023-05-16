const NUM_SONGS = 2;
var songIdx = 0;
var currentSong;

function nextSong() {
  if (currentSong) {
    currentSong.pause();
  }
  if (songIdx = ++songIdx % (NUM_SONGS + 1)) {
    currentSong = new Audio(`./music/${songIdx}.mp3`);
    currentSong.play();
  }
}



const logToConsole = console.log;

function logToTextArea(msg) {
  const logArea = document.getElementById('log-area');
  logArea.value += msg + '\n';
  last10 = logArea.value.split('\n').splice(-11);
  logArea.value = last10.join('\n');
};

console.log = function(msg, alertOnSpecialMsg = true) {
  logToConsole(msg);
  logToTextArea(msg);
  if (alertOnSpecialMsg && !["White to move", "Black to move", "FEN Copied!", "Invalid FEN!"].includes(msg)) {
    alert(msg);
    stopTimer();
  }
};

function clearLog() {
  document.getElementById('log-area').value = "";
  console.clear();
}



var activeTimer;
var timerInterval;

function switchTimer() {
  if (!activeTimer) {
    var gameState = Module.getGameState(fen);
    if (gameState === "White to move") {
      activeTimer = document.querySelector("#black-timer");
    } else if (gameState === "Black to move") {
      activeTimer = document.querySelector("#white-timer");
    }
    startTimer();
  }
  
  activeTimer.classList.remove("active");
  
  if (activeTimer.id === "black-timer") {
    activeTimer = document.querySelector("#white-timer");
  } else {
    activeTimer = document.querySelector("#black-timer");
  }
  
  activeTimer.classList.add("active");
}

function startTimer() {
  timerInterval = setInterval(() => {
    const time = activeTimer.innerHTML.split(':');
    let minutes = parseInt(time[0]);
    let seconds = parseInt(time[1]);

    if (seconds === 0) {
      if (minutes === 0) {
        clearInterval(timerInterval);
        let color = activeTimer.id.split('-')[0];
        let msg = `Timeout: ${color === "white" ? "Black" : "White"} wins`;
        console.log(msg);
        return;
      } else {
        minutes--;
        seconds = 59;
      }
    } else {
      seconds--;
    }

    activeTimer.innerHTML = `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
  }, 1000);
}

function stopTimer() {
  if (activeTimer) {
    clearInterval(timerInterval);
    activeTimer.classList.remove("active");
  }
}

function resetTimer() {
  stopTimer();
  activeTimer = null;
  document.querySelector("#white-timer").innerHTML = "10:00"
  document.querySelector("#black-timer").innerHTML = "10:00"
}



var fen;
var fens = [];
var fenIdx = -1;
var promoting = false;
var promotingSquare = "";

document.addEventListener('DOMContentLoaded', function() {
  document.querySelector("#chessboard").querySelectorAll('*').forEach(function(square) {
    square.addEventListener("mouseup", removeHighlights);
    square.addEventListener("click", click);
    square.addEventListener('dragstart', dragStart);
    square.addEventListener('dragover', dragOver);
    square.addEventListener('drop', drop);
  });
});

Module['onRuntimeInitialized'] = function() {
  replay();
};

function undo() {
  if (fenIdx > 0) {    
    clearLog();
    switchTimer();
    updateBoard(fens[--fenIdx], save = false);
  }
}

function redo() {
  if (fenIdx < fens.length - 1) {
    clearLog();
    switchTimer();
    updateBoard(fens[++fenIdx], save = false);
  }
}

function replay(fen_ = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
  clearLog();
  fens = []
  fenIdx = -1;
  updateBoard(fen_);
  resetTimer();
}

function removeHighlights() {
  document.querySelector('#chessboard').querySelectorAll('*').forEach(function(square) {
    square.classList.remove("highlight");
  });
}

function click(event) {
  if (event.target.id !== promotingSquare) {
    cancelPromotion();
  }
  const targetSquares = Module.getValidTargetSquares(fen, event.target.id).split('\0');
  if (targetSquares[0].length === 2) {
    targetSquares.forEach(function(square) {
      document.getElementById(square).classList.toggle("highlight");
    });
  }
}

function dragStart(event) {
  removeHighlights();
  cancelPromotion();
  click(event);
  event.dataTransfer.setData("text/plain", event.target.id);
}

function dragOver(event) {
  event.preventDefault();
}

function drop(event) {
  event.preventDefault();
  const source = document.getElementById(event.dataTransfer.getData('text/plain'));
  const move = source.id + event.target.id;
  
  if (source.className !== 'empty-square' && Module.isValidMove(fen, move)) {
    if (source.classList.contains("white-pawn") && event.target.id[1] === '8') {
      source.className = "empty-square";
      promotePiece(event.target.id, "white", move);
    } else if (source.classList.contains("black-pawn") && event.target.id[1] === '1') {
      source.className = "empty-square";
      promotePiece(event.target.id, "black", move);
    } else {
      fen = Module.getNextFEN(fen, move);
      updateBoard(fen);
    }
    switchTimer();
  }
}

function updateBoard(fen_, save = true, logGameState = true) {
  cancelPromotion();
  const chessPieces = Module.fenToHtmlClassNames(fen_).split('\0');
  if (chessPieces.length === 64) {
    if (save) {
      fens.push(fen_);
      ++fenIdx;
    }
    if (logGameState) {
      console.log(Module.getGameState(fen_));
    }
    fen = fen_;
    document.getElementById("fen").value = fen;
    document.querySelector('#chessboard').querySelectorAll('*').forEach(function(square, index) {
      square.className = chessPieces[index];
    });
    return true;
  }
  document.getElementById("fen").value = fen;
  return false;
}

function cancelPromotion() {
  if (promoting) {
    promoting = false;
    promotingSquare = ""
    updateBoard(fen, save = false, logGameState = false);
  }
}

function promotePiece(squareId, color, move) {
  promoting = true;
  promotingSquare = squareId;
  document.getElementById(squareId).className = "piece " + color + "-promotion-menu";
  document.querySelector("#" + squareId).addEventListener('click', function(event) {
    const x = event.offsetX;
    const y = event.offsetY;
    const width = event.target.width;
    const height = event.target.height;
    let newPiece = '';
    if (x < width / 2 && y < height / 2) {
      newPiece = "knight";
      move += (color == "white" ? 'N' : 'n');
    } else if (x >= width / 2 && y < height / 2) {
      newPiece = "bishop";
      move += (color == "white" ? 'B' : 'b');
    } else if (x < width / 2 && y >= height / 2) {
      newPiece = "rook";
      move += (color == "white" ? 'R' : 'r');
    } else {
      newPiece = "queen";
      move += (color == "white" ? 'Q' : 'q');
    }
    if (promoting) {
      updateBoard(Module.getNextFEN(fen, move));
      promoting = false;
      promotingSquare = "";
    }
  }, { once: true });
}

function handleFenSubmit(event) {
  event.preventDefault();
  if (!updateBoard(event.target.fen.value)) {
    console.log("Invalid FEN!");
  } else {
    replay(event.target.fen.value);
  }
}

function copyFenToClipboard() {
  navigator.clipboard.writeText(fen);
  console.log("FEN Copied!");
}
