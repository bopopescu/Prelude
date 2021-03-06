CanvasKit - Skia + WebAssembly
==============================

Skia now offers a WebAssembly build for easy deployment of our graphics APIs on
the web.

CanvasKit provides a playground for testing new Canvas and SVG platform APIs,
enabling fast-paced development on the web platform.
It can also be used as a deployment mechanism for custom web apps requiring
cutting-edge features, like Skia's [Lottie
animation](https://skia.org/user/modules/skottie) support.


Features
--------

  - WebGL context encapsulated as an SkSurface, allowing for direct drawing to
    an HTML canvas
  - Core set of Skia canvas/paint/path/text APIs available, see bindings
  - Draws to a hardware-accelerated backend
  - Security tested with Skia's fuzzers

Samples
-------

<style>
  #demo canvas {
    border: 1px dashed #AAA;
    margin: 2px;
  }

  #patheffect, #ink {
    width: 400px;
    height: 400px;
  }

  #sk_legos, #sk_drinks,#sk_party {
    width: 300px;
    height: 300px;
  }

</style>

<div id=demo>
  <h3>An Interactive Path (try mousing over)</h3>
  <canvas id=patheffect width=400 height=400></canvas>
  <canvas id=ink width=400 height=400></canvas>

  <h3>Skottie</h3>
  <canvas id=sk_legos width=300 height=300></canvas>
  <canvas id=sk_drinks width=500 height=500></canvas>
  <canvas id=sk_party width=800 height=800></canvas>
</div>

<script type="text/javascript" charset="utf-8">
(function() {
  // Tries to load the WASM version if supported, shows error otherwise
  let s = document.createElement('script');
  var locate_file = '';
  if (window.WebAssembly && typeof window.WebAssembly.compile === 'function') {
    console.log('WebAssembly is supported!');
    locate_file = 'https://storage.googleapis.com/skia-cdn/canvaskit-wasm/0.0.2/bin/';
  } else {
    console.log('WebAssembly is not supported (yet) on this browser.');
    document.getElementById('demo').innerHTML = "<div>WASM not supported by your browser. Try a recent version of Chrome, Firefox, Edge, or Safari.</div>";
    return;
  }
  s.src = locate_file + 'skia.js';
  s.onload = () => {
  var CanvasKit = null;
  var legoJSON = null;
  var drinksJSON = null;
  var confettiJSON = null;
  CanvasKitInit({
    locateFile: (file) => locate_file + file,
  }).then((CK) => {
    CanvasKit = CK;
    DrawingExample(CanvasKit);
    InkExample(CanvasKit);
     // Set bounds to fix the 4:3 resolution of the legos
    SkottieExample(CanvasKit, 'sk_legos', legoJSON, {fLeft: -50, fTop: 0, fRight: 350, fBottom: 300});
    SkottieExample(CanvasKit, 'sk_drinks', drinksJSON);
    SkottieExample(CanvasKit, 'sk_party', confettiJSON);
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/lego_loader.json').then((resp) => {
    resp.text().then((str) => {
      legoJSON = str;
      SkottieExample(CanvasKit, 'sk_legos', legoJSON);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/drinks.json').then((resp) => {
    resp.text().then((str) => {
      drinksJSON = str;
      SkottieExample(CanvasKit, 'sk_drinks', drinksJSON);
    });
  });

  fetch('https://storage.googleapis.com/skia-cdn/misc/confetti.json').then((resp) => {
    resp.text().then((str) => {
      confettiJSON = str;
      SkottieExample(CanvasKit, 'sk_party', confettiJSON);
    });
  });

  function DrawingExample(CanvasKit) {
    const surface = CanvasKit.getWebGLSurface('patheffect');
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = CanvasKit.currentContext();

    const canvas = surface.getCanvas();

    const paint = new CanvasKit.SkPaint();

    let i = 0;

    let X = 128;
    let Y = 128;

    function drawFrame() {
      const path = starPath(CanvasKit, X, Y);
      CanvasKit.setCurrentContext(context);
      const dpe = CanvasKit.MakeSkDashPathEffect([15, 5, 5, 10], i/5);
      i++;

      paint.setPathEffect(dpe);
      paint.setStyle(CanvasKit.PaintStyle.STROKE);
      paint.setStrokeWidth(5.0 + -3 * Math.cos(i/30));
      paint.setAntiAlias(true);
      paint.setColor(CanvasKit.Color(66, 129, 164, 1.0));

      canvas.clear(CanvasKit.Color(255, 255, 255, 1.0));

      canvas.drawPath(path, paint);
      canvas.flush();
      dpe.delete();
      path.delete();
      window.requestAnimationFrame(drawFrame);
    }
    window.requestAnimationFrame(drawFrame);

    // Make animation interactive
    document.getElementById('patheffect').addEventListener('mousemove', (e) => {
      X = e.offsetX;
      Y = e.offsetY;
    });

    // A client would need to delete this if it didn't go on for ever.
    //paint.delete();
  }

  function InkExample(CanvasKit) {
    const surface = CanvasKit.getWebGLSurface('ink');
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = CanvasKit.currentContext();

    const canvas = surface.getCanvas();

    let paint = new CanvasKit.SkPaint();
    paint.setAntiAlias(true);
    paint.setColor(CanvasKit.Color(0, 0, 0, 1.0));
    paint.setStyle(CanvasKit.PaintStyle.STROKE);
    paint.setStrokeWidth(4.0);
    // This effect smooths out the drawn lines a bit.
    paint.setPathEffect(CanvasKit.MakeSkCornerPathEffect(50));

    let path = new CanvasKit.SkPath();
    path.moveTo(30, 30);
    path.lineTo(60, 80);

    let paths = [path];
    let paints = [paint];

    function drawFrame() {
      CanvasKit.setCurrentContext(context);

      for (let i = 0; i < paints.length && i < paths.length; i++) {
        canvas.drawPath(paths[i], paints[i]);
      }
      canvas.flush();

      window.requestAnimationFrame(drawFrame);
    }

    let hold = false;
    document.getElementById('ink').addEventListener('mousemove', (e) => {
      if (!e.buttons) {
        hold = false;
        return;
      }
      if (hold) {
        path.lineTo(e.offsetX, e.offsetY);
      } else {
        paint = paint.copy();
        paint.setColor(CanvasKit.Color(Math.random() * 255, Math.random() * 255, Math.random() * 255, Math.random() + .2));
        paints.push(paint);
        path = new CanvasKit.SkPath();
        paths.push(path);
        path.moveTo(e.offsetX, e.offsetY);
      }
      hold = true;
    });
    window.requestAnimationFrame(drawFrame);
  }

  function starPath(CanvasKit, X=128, Y=128, R=116) {
    let p = new CanvasKit.SkPath();
    p.moveTo(X + R, Y);
    for (let i = 1; i < 8; i++) {
      let a = 2.6927937 * i;
      p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
    }
    return p;
  }

  function SkottieExample(CanvasKit, id, jsonStr, bounds) {
    if (!CanvasKit || !jsonStr) {
      return;
    }
    const animation = CanvasKit.MakeAnimation(jsonStr);
    const duration = animation.duration() * 1000;
    const size = animation.size();
    let c = document.getElementById(id);
    bounds = bounds || {fLeft: 0, fTop: 0, fRight: size.w, fBottom: size.h};

    const surface = CanvasKit.getWebGLSurface(id);
    if (!surface) {
      console.log('Could not make surface');
    }
    const context = CanvasKit.currentContext();
    const canvas = surface.getCanvas();

    let firstFrame = new Date().getTime();

    function drawFrame() {
      let now = new Date().getTime();
      let seek = ((now - firstFrame) / duration) % 1.0;
      CanvasKit.setCurrentContext(context);
      animation.seek(seek);

      animation.render(canvas, bounds);
      canvas.flush();
      window.requestAnimationFrame(drawFrame);
    }
    window.requestAnimationFrame(drawFrame);
    //animation.delete();
  }
  }
  document.head.appendChild(s);
})();
</script>

Lottie files courtesy of the lottiefiles.com community:
[Lego Loader](https://www.lottiefiles.com/410-lego-loader), [I'm
thirsty](https://www.lottiefiles.com/77-im-thirsty),
[Confetti](https://www.lottiefiles.com/1370-confetti)


Test server
-----------
Test your code on our [CanvasKit Fiddle](https://jsfiddle.skia.org/canvaskit)

Download
--------
Work is underway on an npm download. Check back soon.
