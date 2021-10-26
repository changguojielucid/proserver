/*
*             dP                   oo       dP          oo
*             88                            88
*    .d8888b. 88 dP    dP .d8888b. dP .d888b88 dP   .dP dP dP   .dP .d8888b.
*    88ooood8 88 88    88 88'  `"" 88 88'  `88 88   d8' 88 88   d8' 88'  `88
*    88.  ... 88 88.  .88 88.  ... 88 88.  .88 88 .88'  88 88 .88'  88.  .88
*    `88888P' dP `88888P' `88888P' dP `88888P8 8888P'   dP 8888P'   `88888P'
*    oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
*

*
*    Created:
*
*    Modified:
*
*
*
*
*
*                              Copyright (c) Elucid Bioimaging
*/



#include "EVFirst.h"
#include "EVTargetDef.h"
#include "EVTargetDefine.h"
#include "EVMsgDefine.h"
#include "EVWorkItem.h"

#ifndef tr
	#define tr  
#endif //!tr


bool EVTargetDefine::computeRemainingStages(EVTargetDef* def)
{
	ebLog eblog(Q_FUNC_INFO); eblog << def->getID().toStdString() << std::endl;

	EVLinkedViewers* pEVLinkedViewers = GetEVLinkedViewers();
	ebAssert(pEVLinkedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	//need progress indicated on angular side
	UpdateSenes();

	// verify the starting point and take it through to completion
	if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenSegmentationPreConditions()) {
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation() == NULL) {
			pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Delineating Overall Luminal Surfaces");
			pEVLinkedViewers->SyncViewersToScene();
			pEVLinkedViewers->Render();
			try {
				pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenSegmentation();
			}
			catch (std::exception& e) {
				eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
				PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
				return false;
			}
			logUpdateWithStageSettings("EVTargetDefine::ComputeLumenSegmentation", def, def->getID());
			def->unsavedChanges = true;
			def->lumenSegIDdefine = GetEVLinkedViewers()->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
			clearTargetBeyondCurrentStage(def);
		}

		UpdateSenes();
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PathPreConditions()) {
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() == NULL) {
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Computing Detailed Cross-section Positions");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePath();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					return false;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetPath", def, def->getID());
				def->unsavedChanges = true;
				if (def->initializerIDdefine) {
					pEVLinkedViewers->RemoveVesselTarget(def->initializerIDdefine); // if have a path, don't want to display the initializer too
					def->initializerIDdefine = null_ebID;
				}
				def->pathIDdefine = pEVLinkedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path");
				clearTargetBeyondCurrentStage(def);
			}
		}

		UpdateSenes();
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenPartitionPreConditions()) {
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition() == NULL) {
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Lumen Into Vessels");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenPartition();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					return false;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputeLumenPartition", def, def->getID());
				def->unsavedChanges = true;
				pEVLinkedViewers->RemoveSegmentation4(def->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
				def->lumenSegIDdefine = null_ebID;
				def->lumenPartIDsDefine.clear();
				int j = 0;
				for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition()) {
					def->lumenPartIDsDefine.insert(pEVLinkedViewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition" + std::to_string(j++)));
				}
				clearTargetBeyondCurrentStage(def);
			}
		}

		UpdateSenes();
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->RegistrationPreConditions()) {
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetResampledRegisteredImages() == NULL) {
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Registering Images");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeRegistration();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					return false;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputeRegistration", def, def->getID());
				def->unsavedChanges = true;
				clearTargetBeyondCurrentStage(def);
			}
		}

		UpdateSenes();
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation() == NULL) {
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Delineating Outer Wall Surface");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallSegmentation();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					return false;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallSegmentation", def, def->getID());
				def->unsavedChanges = true;
				def->wallSegIDdefine = pEVLinkedViewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
				pEVLinkedViewers->GetScene()->GetSegmentation4(def->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);
				clearTargetBeyondCurrentStage(def);
			}
		}

		UpdateSenes();
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition() == NULL) {
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Wall Into Vessels");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallPartition();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					return false;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallPartition", def, def->getID());
				def->unsavedChanges = true;
				pEVLinkedViewers->RemoveSegmentation4(def->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
				def->wallSegIDdefine = null_ebID;
				def->wallPartIDsDefine.clear();
				int j = 0;
				for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
					ebID partID = pEVLinkedViewers->AddSegmentation4(partition.GetPointer(), 0, "wallPartition" + std::to_string(j++));
					def->wallPartIDsDefine.insert(partID);
					pEVLinkedViewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);
				}
				clearTargetBeyondCurrentStage(def);
			}
		}

		UpdateSenes();
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->WallThicknessPreConditions()) {
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetWallThickness() == NULL) {
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Computing Wall Thickness Distribution");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeWallThickness();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					return false;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputeWallThickness", def, def->getID());
				def->unsavedChanges = true;
				clearTargetBeyondCurrentStage(def);
			}
		}

		UpdateSenes();
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPreConditions()) {
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegion() == NULL) {
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Establishing Perivascular EVRegion");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegion();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					return false;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputePerivascularRegion", def, def->getID());
				def->unsavedChanges = true;
				clearTargetBeyondCurrentStage(def);
			}
		}
		UpdateSenes();
		if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPartitionPreConditions()) {
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegionPartition() == NULL) {
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Perivascular EVRegion Into Vessels");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegionPartition();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					return false;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputePerivascularRegionPartition", def, def->getID());
				def->unsavedChanges = true;
				clearTargetBeyondCurrentStage(def);
			}

			UpdateSenes();
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CompositionPreConditions()) {
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetComposition() == NULL) {

					//JCG condition
					//  EV_MSG_CONFIRM_
				  //QMessageBox msgBox(this);
				  //msgBox.setText(tr("The current target definitions now enable completion of the calculations."));
				  //msgBox.setInformativeText(tr("Would you like to compute locally, or on the CAPgraph server? Yes means compute locally, No means you prefer to compute on the server."));
				  //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
				  //msgBox.setDefaultButton(QMessageBox::Yes);
				  //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
				   // for ZENITH COMPUTE_ON_PROSERVERALWAYS
				   //		 
					if (true) {
						pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Determining Composition of Wall and Plaque Tissues");
						pEVLinkedViewers->SyncViewersToScene();
						pEVLinkedViewers->Render();
						try {
							// NO LONGER NEED THIS BUT THE PATTERN MIGHT BE USEFUL FOR OTHER PURPOSES LATERif (owner->clinicalJurisdiction != "") // any of the clinical editiuons
							//   pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,false,true,false);
							// else
							pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true, true, true, true, true);
							pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCompositionFilter()->UpdateCalFactorBykVp(GetImageSeries()->kvp.toDouble());
							pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeComposition();
						}
						catch (std::exception& e) {
							eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
							PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
							return false;
						}
						logUpdateWithStageSettings("EVTargetDefine::ComputeComposition", def, def->getID());
						def->unsavedChanges = true;

						//FROM A.3 and later releases, it is no longer needed to display the composition control dialog for the following two reasons.
						//1) MultiKVP calibration (scheduled for A.3 release) will lead to improved composition analysis and hence may not be needed to provide range hint for parameters.
						//2) If still necessary, user can always set the parameter range from the Processing Parameter Dialog. 
						//def->parameters->presentCompositionControl();
						clearTargetBeyondCurrentStage(def);
					}
					else {

						// stop the transition, user will do the computation on the server
						pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
						pEVLinkedViewers->SyncViewersToScene();
						pEVLinkedViewers->Render();
						return false;
					}
				}
			}

			UpdateSenes();
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CapThicknessPreConditions()) {
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCapThickness() == NULL) {
					pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Computing Cap Thickness Distribution");
					pEVLinkedViewers->SyncViewersToScene();
					pEVLinkedViewers->Render();
					try {
						pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeCapThickness();
					}
					catch (std::exception& e) {
						eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
						PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
						return false;
					}
					logUpdateWithStageSettings("EVTargetDefine::ComputeCapThickness", def, def->getID());
					def->unsavedChanges = true;
					clearTargetBeyondCurrentStage(def);
				}
			}

			UpdateSenes();
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ReadingsPreConditions()) {
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings() == NULL) {
					pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Performing Quantitative Calculations");
					pEVLinkedViewers->SyncViewersToScene();
					pEVLinkedViewers->Render();
					try {
						pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeReadings();
					}
					catch (std::exception& e) {
						eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
						PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
						return false;
					}
					logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetReadings", def, def->getID());
					def->unsavedChanges = true;
					def->keyImages.clear(); // anything that caused readings to be re-computed invalidates any saved key images
					clearTargetBeyondCurrentStage(def);
				}
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Target Calculations Completed");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
			}

			UpdateSenes();
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenPartitionPreConditions()) {
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenPartition() == NULL) {
					pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Lumen Into Lesions");
					pEVLinkedViewers->SyncViewersToScene();
					pEVLinkedViewers->Render();
					try {
						pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenPartition();
					}
					catch (std::exception& e) {
						eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
						PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
						return false;
					}
					logUpdateWithStageSettings("EVTargetDefine::ComputeLesionLumenPartition", def, def->getID());
					def->unsavedChanges = true;
					clearTargetBeyondCurrentStage(def);
				}
			}

			UpdateSenes();
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenAndWallPartitionPreConditions()) {
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenAndWallPartition() == NULL) {
					pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Wall Into Lesions");
					pEVLinkedViewers->SyncViewersToScene();
					pEVLinkedViewers->Render();
					try {
						pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenAndWallPartition();
					}
					catch (std::exception& e) {
						eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
						PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
						return false;
					}
					logUpdateWithStageSettings("EVTargetDefine::ComputeLesionLumenAndWallPartition", def, def->getID());
					def->unsavedChanges = true;
					clearTargetBeyondCurrentStage(def);
				}
			}

			UpdateSenes();
			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionPerivascularRegionPartitionPreConditions()) {
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionPerivascularRegionPartition() == NULL) {
					pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning PerivascularRegion Into Lesions");
					pEVLinkedViewers->SyncViewersToScene();
					pEVLinkedViewers->Render();
					try {
						pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionPerivascularRegionPartition();
					}
					catch (std::exception& e) {
						eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
						PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
						return false;
					}
					logUpdateWithStageSettings("EVTargetDefine::ComputeLesionPerivascularRegionPartition", def, def->getID());
					def->unsavedChanges = true;
					clearTargetBeyondCurrentStage(def);
				}
			}

			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionReadingsPreConditions()) {
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionReadings() == NULL) {
					pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Performing Quantitative Calculations for Lesions");
					pEVLinkedViewers->SyncViewersToScene();
					pEVLinkedViewers->Render();
					try {
						pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionReadings();
					}
					catch (std::exception& e) {
						eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
						PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
						return false;
					}
					logUpdateWithStageSettings("EVTargetDefine::ComputeLesionReadings", def, def->getID());
					def->unsavedChanges = true;
					clearTargetBeyondCurrentStage(def);

				}
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Lesion Calculations Completed");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				//progressIndicator.setMaximum(100); // any value will take it down
				return true;
			}
			else {
				///qWarning() << "compute remaining stages was not able to make it all the way through to the lesion readings.";
				pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
				pEVLinkedViewers->SyncViewersToScene();
				pEVLinkedViewers->Render();
				//progressIndicator.setMaximum(100); // any value will take it down
				return false;
			}
			return true;
		}
		else {
			qWarning() << "compute remaining stages was invoked on a target that is not yet viable, skipping.";
			pEVLinkedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
			pEVLinkedViewers->SyncViewersToScene();
			pEVLinkedViewers->Render();
			//progressIndicator.setMaximum(100); // any value will take it down
			return false;
		}
	}


	return false;
}