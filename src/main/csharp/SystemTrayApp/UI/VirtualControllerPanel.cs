﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SystemTrayApp
{
    public partial class VirtualControllerPanel : UserControl, IControllerPanel
    {
        private static ePSMButtonID[] VirtualButtons = new ePSMButtonID[] {
            ePSMButtonID.Virtual_0,
            ePSMButtonID.Virtual_1,
            ePSMButtonID.Virtual_2,
            ePSMButtonID.Virtual_3,
            ePSMButtonID.Virtual_4,
            ePSMButtonID.Virtual_5,
            ePSMButtonID.Virtual_6,
            ePSMButtonID.Virtual_7,
            ePSMButtonID.Virtual_8,
            ePSMButtonID.Virtual_9,
            ePSMButtonID.Virtual_10,
            ePSMButtonID.Virtual_11,
            ePSMButtonID.Virtual_12,
            ePSMButtonID.Virtual_13,
            ePSMButtonID.Virtual_14,
            ePSMButtonID.Virtual_15,
            ePSMButtonID.Virtual_16,
            ePSMButtonID.Virtual_17,
            ePSMButtonID.Virtual_18,
            ePSMButtonID.Virtual_19,
            ePSMButtonID.Virtual_20,
            ePSMButtonID.Virtual_21,
            ePSMButtonID.Virtual_22,
            ePSMButtonID.Virtual_23,
            ePSMButtonID.Virtual_24,
            ePSMButtonID.Virtual_25,
            ePSMButtonID.Virtual_26,
            ePSMButtonID.Virtual_27,
            ePSMButtonID.Virtual_28,
            ePSMButtonID.Virtual_29,
            ePSMButtonID.Virtual_30,
            ePSMButtonID.Virtual_31
        };
        private Dictionary<string, ePSMButtonID> ButtonTable = new Dictionary<string, ePSMButtonID>();

        private VirtualControllerConfig AssignedConfig;

        public VirtualControllerPanel(ControllerConfig config)
        {
            InitializeComponent();

            foreach (ePSMButtonID buttonID in VirtualButtons) {
                string buttonString = Constants.PSMButtonNames[(int)buttonID];
                ButtonTable.Add(buttonString, buttonID);
            }
            InitVirtualButtonComboBox(SystemButtonComboBox);
            InitVirtualButtonComboBox(HMDAlignButtonComboBox);

            AssignedConfig = (VirtualControllerConfig)config;

            VelocityExponentTextField.Text = AssignedConfig.LinearVelocityExponent.ToString();
            VelocityMultiplierTextField.Text = AssignedConfig.LinearVelocityMultiplier.ToString();
            VirtualTumbstickScaleTextField.Text = string.Format("{0}", AssignedConfig.MetersPerTouchpadAxisUnits * 100.0f);
            ThumbstickDeadzoneTextField.Text = AssignedConfig.ThumbstickDeadzone.ToString();
            ExtendYTextField.Text = string.Format("{0}", AssignedConfig.ExtendYMeters * 100.0f);
            ExtendZTextField.Text = string.Format("{0}", AssignedConfig.ExtendZMeters * 100.0f);

            SetVirtualButtonComboBoxValue(SystemButtonComboBox, AssignedConfig.SystemButtonID);
            SetVirtualButtonComboBoxValue(HMDAlignButtonComboBox, AssignedConfig.HMDAlignButtonID);

            SetVirtualAxisComboBoxValue(TouchpadXAxisIndexComboBox, AssignedConfig.VirtualTouchpadXAxisIndex);
            SetVirtualAxisComboBoxValue(TouchpadYAxisIndexComboBox, AssignedConfig.VirtualTouchpadYAxisIndex);
            SetVirtualAxisComboBoxValue(TriggerAxisIndexComboBox, AssignedConfig.SteamVRTriggerAxisIndex);
        }

        private void InitVirtualButtonComboBox(ComboBox combo_box)
        {
            combo_box.DataSource = new BindingSource(ButtonTable, null);
            combo_box.DisplayMember = "Key";
            combo_box.ValueMember = "Value";
        }

        private void SetVirtualButtonComboBoxValue(ComboBox combo_box, ePSMButtonID button_id)
        {
            combo_box.SelectedIndex = (int)button_id - (int)ePSMButtonID.Virtual_0;
        }

        private ePSMButtonID GetVirtualButtonComboBoxValue(ComboBox combo_box)
        {
            return ((KeyValuePair<string, ePSMButtonID>)combo_box.SelectedItem).Value;
        }

        private void SetVirtualAxisComboBoxValue(ComboBox combo_box, int axis_id)
        {
            combo_box.SelectedIndex = axis_id + 1;
        }

        private int GetVirtualAxisComboBoxValue(ComboBox combo_box)
        {
            return combo_box.SelectedIndex - 1;
        }

        private void DisableAlignmentGestureCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            AssignedConfig.DisableAlignmentGesture = DisableAlignmentGestureCheckBox.Checked;
        }

        private void TouchpadPressDelayCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            AssignedConfig.DelayAfterTouchpadPress = TouchpadPressDelayCheckBox.Checked;
        }

        private void ZRotate90CheckBox_CheckedChanged(object sender, EventArgs e)
        {
            AssignedConfig.ZRotate90Degrees = ZRotate90CheckBox.Checked;
        }

        private void AddNewMappingButton_Click(object sender, EventArgs e)
        {
            TouchpadMappingsLayoutPanel.Controls.Add(new ButtonMapping(VirtualButtons, ePSMButtonID.PS, eEmulatedTrackpadAction.None));
        }

        private void TouchpadXAxisIndexComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            AssignedConfig.VirtualTouchpadXAxisIndex = GetVirtualAxisComboBoxValue(TouchpadXAxisIndexComboBox);
        }

        private void TouchpadYAxisIndexComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            AssignedConfig.VirtualTouchpadYAxisIndex = GetVirtualAxisComboBoxValue(TouchpadYAxisIndexComboBox);
        }

        private void TriggerAxisIndexComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            AssignedConfig.SteamVRTriggerAxisIndex = GetVirtualAxisComboBoxValue(TriggerAxisIndexComboBox);
        }

        private void HMDAlignButtonComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            AssignedConfig.HMDAlignButtonID = GetVirtualButtonComboBoxValue(HMDAlignButtonComboBox);
        }

        private void SystemButtonComboBox_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            AssignedConfig.SystemButtonID = GetVirtualButtonComboBoxValue(SystemButtonComboBox);
        }

        public void Reload()
        {
            throw new NotImplementedException();
        }

        public void Save()
        {
            throw new NotImplementedException();
        }
    }
}