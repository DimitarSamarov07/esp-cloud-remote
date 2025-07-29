import { Router } from "express";
import * as AirConditionerController from "../controllers/AirConditioner.controller.ts";

const router = Router();

router.get('/:deviceId', AirConditionerController.getAirConditionerByID);
router.post('/', AirConditionerController.createAirConditioner);
router.put('/update', AirConditionerController.updateAirConditionerByID);
router.delete('/:deviceId', AirConditionerController.deleteAirConditionerByID);

export default router;
