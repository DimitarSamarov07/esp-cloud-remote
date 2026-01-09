import {Router} from "express";
import * as AirConditionerController from "../controllers/AirConditioner.controller.ts";

const router = Router();

router.get('/:deviceID', AirConditionerController.getAirConditionerByID);
router.post('/', AirConditionerController.createAirConditioner);
router.put('/:deviceID', AirConditionerController.updateAirConditionerByID);
router.delete('/:deviceID', AirConditionerController.deleteAirConditionerByID);

export default router;
