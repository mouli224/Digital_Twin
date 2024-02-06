<template>
  <div ref="container">
    <div id="data">
      <h2>Height:</h2>
      <p>{{ height }} <span>mm</span></p>
      <h2>Weight:</h2>
      <p :class="{red:weight>1000}">{{ weight }} <span>g</span></p>
    </div>
  </div>
</template>

<script>
import URDFLoader from "urdf-loader";
import {
  WebGLRenderer,
  PerspectiveCamera,
  Scene,
  Mesh,
  PlaneBufferGeometry,
  ShadowMaterial,
  DirectionalLight,
  PCFSoftShadowMap,
  sRGBEncoding,
  Color,
  AmbientLight,
  // Box3,
  // Vector3,
  LoadingManager,
  // MeshPhongMaterial,
} from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";
import io from "socket.io-client";
let thescene;
export default {
  name: "App",

  data() {
    return {
      scene: null,
      camera: null,
      renderer: null,
      robot: null,
      controls: null,
      socket: io("http://localhost:3000", { transports: ["websocket"] }),
      height: 0.0,
      angle: 0.0,
      weight: 0.0,
      max_weight: 1000,
    };
  },

  components: {},
  mounted() {
    this.init();
    this.animate();
    let vm = this;
    this.socket.on("jack", (data) => {
      var buffer = new Uint8Array(data);
      var str = String.fromCharCode.apply(null, buffer);
      const values = str.split(";");
      vm.angle = Number(values[0]);
      vm.height = Number(values[1]);
      vm.weight = Number(values[2]) < 0 ? 0 : Number(values[2]);
      vm.robot.setJointValue("screw", vm.height / 1500);
      vm.robot.setJointValue("head", vm.angle/100);
      var color_ratio = vm.weight/vm.max_weight;
      if(color_ratio>1.0)color_ratio = 1.0;
      var hue  =(1 - color_ratio) * 120
      var color = new Color(`hsl(${hue}, 100%, 50%)`);
      thescene.traverse(function (child) {
        if (child.isMesh) {
          if (child.parent.isURDFVisual) {
            if (child.parent.parent.name == "load_cell_1") {
              child.material.color.setHex("0x"+color.getHexString());
            }
          }
        }
      });
    });
  },
  methods: {
    animate() {
      let vm = this;
      requestAnimationFrame(vm.animate);
      vm.controls.update();
      vm.renderer.render(thescene, vm.camera);
    },
    init() {
      let vm = this;
      thescene = new Scene();
      thescene.background = new Color(0x263238);

      vm.camera = new PerspectiveCamera(3, window.devicePixelRatio, 0.1, 10000);
      vm.camera.position.set(6, 5, -20);
      vm.camera.translateZ(12);
      vm.camera.lookAt(0, 0, 0);

      vm.renderer = new WebGLRenderer({
        antialias: false,
      });
      vm.renderer.setPixelRatio(window.devicePixelRatio);
      vm.renderer.setSize(window.innerWidth, window.innerHeight);
      vm.renderer.outputEncoding = sRGBEncoding;
      vm.renderer.shadowMap.enabled = true;
      vm.renderer.shadowMap.type = PCFSoftShadowMap;
      this.$refs.container.appendChild(vm.renderer.domElement);

      const directionalLight = new DirectionalLight(0xffffff, 1.0);
      directionalLight.castShadow = true;
      directionalLight.shadow.mapSize.setScalar(1024);
      directionalLight.position.set(5, 30, 5);
      thescene.add(directionalLight);

      const ambientLight = new AmbientLight(0xffffff, 0.2);
      thescene.add(ambientLight);

      const ground = new Mesh(
        new PlaneBufferGeometry(),
        new ShadowMaterial({ opacity: 0.25 })
      );
      ground.rotation.x = -Math.PI / 2;
      ground.scale.setScalar(30);
      ground.receiveShadow = true;
      thescene.add(ground);

      vm.controls = new OrbitControls(vm.camera, vm.renderer.domElement);
      vm.controls.minDistance = 1;
      vm.controls.target.y = 0;
      vm.controls.update();
      vm.renderer.render(thescene, vm.camera);

      const manager = new LoadingManager();
      const loader = new URDFLoader(manager);

      loader.load(
        "./jack_description/jack.URDF", // The path to the URDF within the package OR absolute
        (robot) => {
          robot.rotation.x = -Math.PI / 2;
          robot.rotation.z = -1;
          vm.robot = robot;
          thescene.add(robot);
        }
      );
      manager.onLoad = () => {
        let vm = this;
        vm.robot.traverse((c) => {
          c.castShadow = true;
        });

        vm.robot.updateMatrixWorld(true);
        vm.scene = thescene;
        vm.renderer.render(thescene, vm.camera);
        // thescene.traverse(function (child) {
        //     if(child.isMesh){
        //       if(child.parent.isURDFVisual){
        //         if(child.parent.parent.name == "load_cell_1"){
        //           child.material.color.setHex(0xff0000);
        //         }
        //       }
        //     }

        // });
        // setInterval(() => {

        // vm.robot.setJointValue("screw", 0.06);

        // }, 5000);
      };
    },
  },
};
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
canvas {
  position: absolute;
  top: 10vh;
  width: 100vh !important;
  height: 80vh !important;
}
body {
  background-color: #263238;
}
#data {
  position: absolute;
  z-index: 99;
  top: 30%;
  right: 20%;
  min-width: 17vw;
}
h2 {
  margin: 0 0 5px 0;
  font-size: 40px;
  color: white;
  opacity: 0.7;
}
p {
  margin: 0 0 15px 0;
  font-size: 50px;
  font-weight: bold;
  color: RGB(255, 180, 0);
}
p > span {
  margin: 0 0 0 5px;
  font-size: 30px;
  color: white;
  opacity: 0.7;
}
.red{
  color: RGB(255, 0, 0);
}
.yellow{
  color: RGB(255, 180, 0);
}
</style>
