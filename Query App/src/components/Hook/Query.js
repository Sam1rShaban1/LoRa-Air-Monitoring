import React, { useContext } from 'react';
import { Form, Input, Row, Col, Button, Select } from 'antd';
import { QosOption } from './index';

const Query = ({ publish, sub }) => {
  const [form] = Form.useForm();
  const qosOptions = useContext(QosOption);

  const onFinish = (values) => {
    const { qos, node } = values;

    const pubTopic = "from-server/" + node;
    const subTopic = "to-server/" + node;

    console.log(subTopic);

    const pubValues = { ...values, pubTopic };
    const subValues = { subTopic, qos };

    sub(subValues);
    publish(pubValues);
  };

  const recordP = {
    node: '',
    qos: 0,
  };

  const labelStyle = { color: 'white' }; // Style for labels

  const QueryForm = (
    <Form
      layout="vertical"
      name="basic"
      form={form}
      initialValues={recordP}
      onFinish={onFinish}
    >
      <Row gutter={20}>
        <Col span={12}>
          <Form.Item
            label={<span style={labelStyle}>Node</span>}
            name="node"
          >
            <Input />
          </Form.Item>
        </Col>
        <Col span={12}>
          <Form.Item
            label={<span style={labelStyle}>QoS</span>}
            name="qos"
          >
            <Select options={qosOptions} />
          </Form.Item>
        </Col>
        <Col span={24}>
          <Form.Item
            label={<span style={labelStyle}>Query</span>}
            name="payload"
          >
            <Input.TextArea />
          </Form.Item>
        </Col>
        <Col span={8} offset={16}>
          <Form.Item>
            <Button type="primary" htmlType="submit">
              Query
            </Button>
          </Form.Item>
        </Col>
      </Row>
    </Form>
  );

  return (
    <>
      <h2>QUERY</h2>
      {QueryForm}
    </>
  );
};

export default Query;
