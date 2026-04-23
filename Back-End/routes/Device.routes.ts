import {Router} from "express";
import * as DeviceController from "../controllers/Device.controller.ts";

const router = Router();

router.post('/changeWifi', DeviceController.changeWifi)
router.post('/register', DeviceController.registerDevice)
router.post('validateCredentials', DeviceController.validateCredentials)